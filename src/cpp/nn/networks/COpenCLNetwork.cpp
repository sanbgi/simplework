
#include "network.h"

#define CL_HPP_TARGET_OPENCL_VERSION 200
#include "cl/cl2.hpp"

#include <map>
#include <string>
#include <fstream> 
#include <iostream>
using namespace sw;
using namespace std;

static SCtx sCtx("COpenCLNetwork");
class COpenCLNetwork : public CObject, public INnNetwork, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.OpenCLNetwork"; }

public://INnOpenCLNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);
    int update(const STensor& spBatchInDeviation);

private:
    template<typename Q>
    int evalT(const STensor& spBatchIn, STensor& spBatchOut);

    template<typename Q>
    int deviaT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);

    template<typename Q>
    int updateT(const STensor& spBatchInDeviation);

private:
    PNnSolveGraph m_sSolveGraph;
    string m_strOptimizer;

private:
    //从计算图中获取的求解信息
    struct PSolveGraphInfos {
        PID idType;
        SDimension spInDimension;
        SDimension spOutDimension;

        // 计算变量
        struct PSolveVar {
            ENnVariableType type;//ENnVariableType
            int size;
            void* data; //存储状态及权重值的指针
        };

        // 计算指令
        struct PSolveInstruct {
            PSolveFunc solver;
            cl::Kernel eval;
            cl::Kernel devia;
            PNnAtomOperatorArgs args;
        };
        cl::Kernel kZero;

        //解算参数列表
        vector<PSolveVar> arrVars;
        //解算步骤列表
        vector<PSolveInstruct> arrInstructs;

        int iInVar;
        int iOutVar;
        int nSumSize[EVMax];
        int nMaxParameterSize;
    
        int nInputTensorSize;
        int nOutputTensorSize;
        SOptimizer spOptimizer;
    };
    CTaker<PSolveGraphInfos*> m_spSolveGraphInfos;

public:
    int initNetwork(PID idType);

private:
    static int getProgram(string name, cl::Program& program) {
        string filename = "opencl/" + name;
        ifstream in(filename.c_str(), std::ios_base::binary);
        if(!in.good()) {
            return sCtx.error(("读取内核文件错误，文件名："+filename).c_str());
        }

        // get file length
        in.seekg(0, std::ios_base::end);
        size_t length = in.tellg();
        in.seekg(0, std::ios_base::beg);

        // read program source
        std::vector<char> data(length + 1);
        in.read(data.data(), length);
        data[length] = 0;

        cl::Program vectorProgram(data.data());
        try {
            cl_int ret = vectorProgram.build("-cl-std=CL2.0");
            if( ret != CL_SUCCESS) {
                return sCtx.error(("编译内核文件错误，文件名："+filename).c_str());
            }
        }
        catch (...) {
            // Print build info for all devices
            cl_int buildErr = CL_SUCCESS;
            auto buildInfo = vectorProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
            for (auto &pair : buildInfo) {
                std::cerr << pair.second << std::endl << std::endl;
            }
            return sCtx.error(("编译内核文件错误，文件名："+filename).c_str());
        }
        program = vectorProgram;
        return sCtx.success();
    }

    template<typename Q>
    static int getKernel(string name, string ext, cl::Kernel& kEval, cl::Kernel& kDevia) {
        static map<string,cl::Program> s_mapProgrames;
        auto it = s_mapProgrames.find(name);
        if( it != s_mapProgrames.end() ) {
            kEval = cl::Kernel(it->second, (name + "_eval").c_str());
            kDevia = cl::Kernel(it->second, (name + "_devia").c_str());
            return sCtx.success();
        }

        cl::Program program;
        if( getProgram(name+ext, program) != sCtx.success() ) {
            return sCtx.error();
        }
        kEval = cl::Kernel(program, (name + "_eval").c_str());
        kDevia = cl::Kernel(program, (name + "_devia").c_str());
        s_mapProgrames[name] = program;
        return sCtx.success();
    }

    template<typename Q>
    static int getBasicKernel(string name, cl::Kernel& k) {
        static bool s_Initialized = false;
        static cl::Program s_programBasic;
        if( !s_Initialized ) {
            
            unsigned long idType = CBasicData<Q>::getStaticType();
            if( idType == CBasicData<float>::getStaticType()) {
                const string ext = "_float.cl";
                if( getProgram("basic"+ext, s_programBasic) != sCtx.success() ) {
                    return sCtx.error();
                }
            }else if( idType == CBasicData<double>::getStaticType() ) {
                const string ext = "_double.cl";
                if( getProgram("basic"+ext, s_programBasic) != sCtx.success() ) {
                    return sCtx.error();
                }
            }else{
                return sCtx.error("数据类型目前只支持float、double");
            }
            s_Initialized = true;
        }
        k = cl::Kernel(s_programBasic, name.c_str());;
        return sCtx.success();
    }
};

int COpenCLNetwork::__initialize(const PData* pData){
    const PNnNetwork* pNet = CData<PNnNetwork>(pData);
    if(pNet == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    if( CNnVariableSolver::solveNetwork(pNet, &m_sSolveGraph) != sCtx.success()) {
        return sCtx.error("解算网络单元错误");
    }

    static bool s_initialized = false;
    static std::once_flag s_initializedFlag;
    std::call_once(s_initializedFlag, [&]{
        // Filter for a 2.0 platform and set it as the default
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        cl::Platform plat;
        for (auto &p : platforms) {
            std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
            if (platver.find("OpenCL 2.") != std::string::npos) {
                plat = p;
                break;
            }
        }
        if (plat() == 0)  {
            sCtx.error("No OpenCL 2.0 platform found.");
            return;
        }
        cl::Platform newP = cl::Platform::setDefault(plat);
        if (newP != plat) {
            sCtx.error("Error setting default platform.");
            return;
        }
        s_initialized = true;
    });
    if(s_initialized == false) {
        return sCtx.error("初始化OpenCL失败");
    } 
    return sCtx.success();
}

int COpenCLNetwork::initNetwork(PID idType) {
    if(m_spSolveGraphInfos) {
        if(idType == m_spSolveGraphInfos->idType) {
            return sCtx.success();
        }else{
            return sCtx.error("数据类型与初始化时不一致，暂不支持");
        }
    }

    CTaker<PSolveGraphInfos*> taker(new PSolveGraphInfos, [](PSolveGraphInfos* pCtx){
        delete pCtx;
    });
    PSolveGraphInfos& solveCtx = *taker;
    for(int i=0; i<EVMax; i++) {
        solveCtx.nSumSize[i] = 0;
    }

    //
    // 更新计算变量数组
    //
    vector<PSolveGraphInfos::PSolveVar>& arrVars = solveCtx.arrVars;
    for(auto itVar : m_sSolveGraph.arrVars) {
        SNnInternalVariable spToSolveVar = itVar;
        if(!spToSolveVar) {
            return sCtx.error("不认识的变量类型");
        }
        PSolveGraphInfos::PSolveVar solveVar;
        solveVar.size = spToSolveVar->getSize();
        solveVar.type = spToSolveVar->getVariableType();
        solveVar.data = spToSolveVar->getData(idType);
        solveCtx.nSumSize[solveVar.type] += solveVar.size;
        arrVars.push_back(solveVar);
    }

    //
    // 更新计算步骤数组
    //
    int nMaxParameterSize = 0;
    vector<PSolveGraphInfos::PSolveInstruct>& arrInstructs = solveCtx.arrInstructs;
    auto itParameter = m_sSolveGraph.arrOperatorArgs.begin();
    auto itOp = m_sSolveGraph.arrOperators.begin();
    while(itParameter != m_sSolveGraph.arrOperatorArgs.end()) {
        PNnAtomOperatorArgs spOp = *itParameter;
        PSolveGraphInfos::PSolveInstruct solveParameter;
        solveParameter.args = spOp;
        (*itOp)->prepareSolver({idType,PSolveCtx::CPU}, solveParameter.solver);
        if(solveParameter.solver.nParamterSize > nMaxParameterSize) {
            nMaxParameterSize = solveParameter.solver.nParamterSize;
        }

        string name = (*itOp)->getName();
        if(idType == CBasicData<float>::getStaticType()) {
            const string ext = "_float.cl";
            if( getKernel<float>(name,ext,solveParameter.eval, solveParameter.devia) != sCtx.success() ) {
                return sCtx.error("获取计算内核错误");
            }
        }else if( idType == CBasicData<double>::getStaticType() ) {
            const string ext = "_double.cl";
            if( getKernel<float>(name,ext,solveParameter.eval, solveParameter.devia) != sCtx.success() ) {
                return sCtx.error("获取计算内核错误");
            }
        }else{
            return sCtx.error("数据类型目前只支持float、double");
        }

        arrInstructs.push_back(solveParameter);
        itParameter++, itOp++;
    }

    //
    // 获取置零内核
    //
    if( getBasicKernel<float>("zero",solveCtx.kZero) != sCtx.success() ) {
        return sCtx.error("获取计算内核错误");
    }

    //
    // 创建优化器
    //
    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, solveCtx.spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    SDimension spInDimension = m_sSolveGraph.arrVars[m_sSolveGraph.iInVar].dimension();
    SDimension spOutDimension = m_sSolveGraph.arrVars[m_sSolveGraph.iOutVar].dimension();
    solveCtx.iInVar = m_sSolveGraph.iInVar;
    solveCtx.iOutVar = m_sSolveGraph.iOutVar;
    solveCtx.nMaxParameterSize = nMaxParameterSize;
    solveCtx.idType = idType;
    solveCtx.nInputTensorSize = spInDimension.dataSize();
    solveCtx.nOutputTensorSize = spOutDimension.dataSize();
    solveCtx.spInDimension = spInDimension;
    solveCtx.spOutDimension = spOutDimension;
    m_spSolveGraphInfos.take(taker);
    return sCtx.success();
}

int COpenCLNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    PID idType = spBatchIn.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }
    return sCtx.error("不支持的数据类型"); 
}

template<typename Q>
int COpenCLNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {
    cl_int ret = 0;
    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;

    SDimension spInDimension = spBatchIn.dimension();
    int nBatchs = *spInDimension.data();
    int nInputSize = spInDimension.dataSize();
    if(nInputSize != nBatchs * solveCtx.nInputTensorSize) {
        return sCtx.error("输入张量尺寸和网络需要的尺寸不匹配");
    }
    
    //
    // 准备计算缓冲区
    //
    STensor spOpSolveBuffer;
    if( STensor::createVector<Q>(spOpSolveBuffer, solveCtx.nSumSize[EVOperator] * nBatchs) != sCtx.success()) {
        return sCtx.error("创建计算缓冲区失败");
    }

    struct PSolveVector {
        int size;
        cl::Buffer data;
    };
    vector<PSolveVector> solveVars(solveCtx.arrVars.size());
    {
        PSolveVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                pItVec->size = pItVar->size * nBatchs;
                pItVec->data = cl::Buffer(CL_MEM_READ_WRITE, pItVec->size * sizeof(Q));
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                pItVec->size = pItVar->size;
                pItVec->data = cl::Buffer(CL_MEM_READ_ONLY, pItVec->size  * sizeof(Q));
                if( ret = cl::copy((Q*)pItVar->data, (Q*)pItVar->data+pItVar->size, pItVec->data) != CL_SUCCESS ) {
                    return sCtx.error("OpenCL拷贝数据错误");
                }
                break;

            case ENnVariableType::EVInput:
                {
                    pItVec->size = pItVar->size * nBatchs;
                    pItVec->data = cl::Buffer(CL_MEM_READ_ONLY, pItVec->size * sizeof(Q));
                    Q* pIn = spBatchIn.data<Q>();
                    if( ret = cl::copy(pIn, pIn + pItVec->size, pItVec->data) != CL_SUCCESS ){
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PSolveVector* pVec;
    cl::Event event;
    cl::Buffer bufferParameters(CL_MEM_READ_ONLY, solveCtx.nMaxParameterSize);
    for(auto& instruct : solveCtx.arrInstructs ) {
        if(instruct.solver.nParamterSize>0) {
            unsigned char* pParameter = (unsigned char*)instruct.solver.pParameterData;
            if( ret = cl::copy(pParameter, pParameter + instruct.solver.nParamterSize, bufferParameters) != CL_SUCCESS ) {
                return sCtx.error("OpenCL拷贝数据错误");
            }
        }
        instruct.eval.setArg(0, bufferParameters);
        instruct.eval.setArg(1, nBatchs);
        for(int j=0; j<instruct.args.nInVars; j++) {
            pVec = &solveVars[instruct.args.pInVars[j]];
            instruct.eval.setArg(j*2+2, pVec->size);
            instruct.eval.setArg(j*2+3, pVec->data);
        }
        pVec = &solveVars[instruct.args.iOutVar];
        instruct.eval.setArg(instruct.args.nInVars*2+2, pVec->size);
        instruct.eval.setArg(instruct.args.nInVars*2+3, pVec->data);
        cl::NDRange globalRange;
        switch(instruct.solver.eClRange) {
            case PSolveFunc::PBatchAndOut:
                globalRange = cl::NDRange(nBatchs, pVec->size/nBatchs);
                break;

            case PSolveFunc::PBatch:
                globalRange = cl::NDRange(nBatchs);
                break;

            case PSolveFunc::PCustomer:
                globalRange = cl::NDRange(instruct.solver.nCustomerRange);
                break;

            case PSolveFunc::POut:
                globalRange = cl::NDRange(pVec->size);
                break;

            default:
                globalRange = cl::NDRange(1);
                break;
        }
        cl_int ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
            instruct.eval,
            cl::NullRange,
            globalRange,
            cl::NullRange,
            nullptr,
            &event
        );
        if(ret != CL_SUCCESS) {
            return sCtx.error("OpenCL计算错误");
        }
    }
    event.wait();

    //  
    // 准备计算指针
    //  1，输入变量的数据指针指向输入数据
    //  2，运算变量指针指向缓冲区
    //
    int iOffset = 0;
    {
        Q* pOpSolvedBuffer = spOpSolveBuffer.data<Q>();
        Q* pItBuffer = pOpSolvedBuffer;
        PSolveVector* pItVec = solveVars.data();
        PSolveVector* pOutVec = pItVec + solveCtx.iOutVar;
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                cl::copy(pItVec->data, pItBuffer, pItBuffer + pItVec->size);
                if(pItVec == pOutVec) {
                    iOffset = (int)(pItBuffer - pOpSolvedBuffer);
                }
                pItBuffer+=pItVec->size;
                break;
            }
        }
    }

    SDimension spOutDim = solveCtx.spOutDimension.upHighDimension(nBatchs);
    return CNnResizeTensor::createResizeTensor({spOpSolveBuffer, spOutDim, iOffset, spBatchIn}, spBatchOut);
}

int COpenCLNetwork::devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    int idType = spBatchOut.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(spBatchOutDeviation.type() != idType || spBatchOutDeviation.size() != spBatchOut.size() ) {
        return sCtx.error("偏差数据类型或尺寸错误");
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return deviaT<double>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return deviaT<float>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

static int zeroBuffer(cl::Kernel k, cl::Buffer buffer, int size) {
    k.setArg(0, buffer);
    cl_int ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
        k,
        cl::NullRange,
        cl::NDRange(size),
        cl::NullRange
    );
    if(ret != CL_SUCCESS) {
        return sCtx.error("OpenCL计算错误");
    }
    return sCtx.success();
}

template<typename Q>
int COpenCLNetwork::deviaT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    cl_int ret = 0;
    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;

    SDimension spBatchOutDimension = spBatchOut.dimension();
    int nBatchs = *spBatchOutDimension.data();
    int nOutputSize = spBatchOutDimension.dataSize();
    if(nOutputSize != nBatchs * solveCtx.nOutputTensorSize) {
        return sCtx.error("输出张量尺寸和网络需要的尺寸不匹配");
    }

    SNnResizeTensor spResizeOut = spBatchOut;
    if( !spResizeOut ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnResizeTensor sResizeTensor;
    spResizeOut->getResizeData(sResizeTensor);
    spBatchIn = sResizeTensor.spExtra;

    struct PSolveDeviaVector {
        int size;
        cl::Buffer data;
        cl::Buffer devia;
    };
    vector<PSolveDeviaVector> solveVars(solveCtx.arrVars.size());
    {
        Q* pItOpVar = sResizeTensor.spSrc.data<Q>();
        PSolveDeviaVector* pItVec = solveVars.data();
        PSolveDeviaVector* pOutVar = solveVars.data() + solveCtx.iOutVar;
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            pItVec->size = pItVar->size;
            switch(pItVar->type) {
                case ENnVariableType::EVOperator:
                case ENnVariableType::EVInput:
                    pItVec->size *= nBatchs;
                    break;
            }
            int nSize = pItVec->size * sizeof(Q);
            pItVec->data = cl::Buffer(CL_MEM_READ_ONLY, nSize);
            pItVec->devia = cl::Buffer(CL_MEM_READ_WRITE, nSize);
            if( zeroBuffer(solveCtx.kZero, pItVec->devia, nSize) != sCtx.success() ){
                return sCtx.error("OpenCL初始化数据错误");
            }
        
            switch(pItVar->type) {
            case ENnVariableType::EVOperator:
                if(pItVec == pOutVar) {
                    Q* pOutDevia = spBatchOutDeviation.data<Q>();
                    if( ret = cl::copy(pOutDevia, pOutDevia+pItVec->size, pItVec->devia) != CL_SUCCESS ) {
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }else{
                    if( ret = cl::copy(pItOpVar, pItOpVar+pItVec->size, pItVec->data) != CL_SUCCESS ) {
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }
                pItOpVar += pItVec->size;
                break;

            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
                if( ret = cl::copy((Q*)pItVar->data, (Q*)pItVar->data+pItVar->size, pItVec->data) != CL_SUCCESS ) {
                    return sCtx.error("OpenCL拷贝数据错误");
                }
                break;

            case ENnVariableType::EVInput:
                {
                    Q* pIn = spBatchIn.data<Q>();
                    if( ret = cl::copy(pIn, pIn + pItVec->size, pItVec->data) != CL_SUCCESS ){
                        return sCtx.error("OpenCL拷贝数据错误");
                    }
                }
                break;
            }
        }
    }

    //
    // 遍历计算序列并执行
    //
    PSolveDeviaVector* pVec;
    cl::Event event;
    cl::Buffer bufferParameters(CL_MEM_READ_ONLY, solveCtx.nMaxParameterSize);
    for(auto itSolver=solveCtx.arrInstructs.rbegin(); itSolver != solveCtx.arrInstructs.rend(); itSolver++) {

        //准备输入输出计算参数
        PSolveGraphInfos::PSolveInstruct& instruct = *itSolver;

        if(instruct.solver.nParamterSize>0) {
            unsigned char* pParameter = (unsigned char*)instruct.solver.pParameterData;
            if( ret = cl::copy(pParameter, pParameter + instruct.solver.nParamterSize, bufferParameters) != CL_SUCCESS ) {
                return sCtx.error("OpenCL拷贝数据错误");
            }
        }
        instruct.devia.setArg(0, bufferParameters);
        instruct.devia.setArg(1, nBatchs);
        for(int j=0; j<instruct.args.nInVars; j++) {
            pVec = &solveVars[instruct.args.pInVars[j]];
            instruct.devia.setArg(j*3+2, pVec->size);
            instruct.devia.setArg(j*3+3, pVec->data);
            instruct.devia.setArg(j*3+4, pVec->devia);
        }
        pVec = &solveVars[instruct.args.iOutVar];
        instruct.devia.setArg(instruct.args.nInVars*3+2, pVec->size);
        instruct.devia.setArg(instruct.args.nInVars*3+3, pVec->devia);
        cl::NDRange globalRange;
        switch(instruct.solver.eClRange) {
            case PSolveFunc::PBatchAndOut:
                globalRange = cl::NDRange(nBatchs, pVec->size/nBatchs);
                break;

            case PSolveFunc::PBatch:
                globalRange = cl::NDRange(nBatchs);
                break;

            case PSolveFunc::PCustomer:
                globalRange = cl::NDRange(instruct.solver.nCustomerRange);
                break;

            case PSolveFunc::POut:
                globalRange = cl::NDRange(pVec->size);
                break;

            default:
                globalRange = cl::NDRange(1);
                break;
        }
        cl_int ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
            instruct.devia,
            cl::NullRange,
            globalRange,
            cl::NullRange,
            nullptr,
            &event
        );
        if(ret != CL_SUCCESS) {
            return sCtx.error("OpenCL计算错误");
        }
    }
    event.wait();

    //
    // 拷贝权重核输入偏导数
    //
    int nWeights = solveCtx.nSumSize[EVWeight];
    Q* pWeightDeviaBuffer = (Q*)solveCtx.spOptimizer->getDeviationPtr(nWeights);
    if( int errCode = STensor::createTensor(spBatchInDeviation, spBatchIn.dimension(), solveCtx.idType, spBatchIn.size()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }
    {
        Q* pBatchInDevia = spBatchInDeviation.data<Q>();
        Q* pItWeightBuffer = pWeightDeviaBuffer;
        PSolveDeviaVector* pItVec = solveVars.data();
        for(auto pItVar = solveCtx.arrVars.begin(); pItVar != solveCtx.arrVars.end(); pItVar++, pItVec++ ){
            switch(pItVar->type) {
            case ENnVariableType::EVWeight:
                cl::copy(pItVec->devia, pItWeightBuffer, pItWeightBuffer + pItVec->size);
                pItWeightBuffer += pItVec->size;
                break;

            case ENnVariableType::EVInput:
                cl::copy(pItVec->devia, pBatchInDevia, pBatchInDevia + pItVec->size);
                break;
            }
        }
    }

    solveCtx.spOptimizer->updateDeviation(nBatchs);
    STensor spWeightDevia = STensor::createVector<Q>(nWeights, pWeightDeviaBuffer);
    return CNnResizeTensor::createResizeTensor({spBatchInDeviation, spBatchInDeviation.dimension(), 0, spWeightDevia}, spBatchInDeviation);
}

int COpenCLNetwork::update(const STensor& spBatchInDeviation) {
    int idType = spBatchInDeviation.type();
    if( initNetwork(idType) != sCtx.success() ) {
        return sCtx.error("网络初始化失败");
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return updateT<double>(spBatchInDeviation);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return updateT<float>(spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

template<typename Q>
int COpenCLNetwork::updateT(const STensor& spBatchInDeviation) {
    SNnResizeTensor spResizeDevia = spBatchInDeviation;
    if( !spResizeDevia ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnResizeTensor sResizeTensor;
    spResizeDevia->getResizeData(sResizeTensor);
    STensor spWeightDevia = sResizeTensor.spExtra;

    PSolveGraphInfos& solveCtx = *m_spSolveGraphInfos;
    int nWeights = solveCtx.nSumSize[EVWeight];
    if(spWeightDevia.size() != nWeights) {
        return sCtx.error("数据错误，无法用于更新权重");
    }

    Q* pItWeightDevia = spWeightDevia.data<Q>();
    vector<PSolveGraphInfos::PSolveVar>::iterator itVar = solveCtx.arrVars.begin();
    while(itVar != solveCtx.arrVars.end() ) {
        switch(itVar->type) {
        case ENnVariableType::EVWeight:
            {
                Q* pItDevia = pItWeightDevia;
                Q* pItData = (Q*)(itVar->data);
                Q* pDataEnd = pItData + itVar->size;
                while(pItData < pDataEnd) {
                    *pItData -= *pItDevia;
                    if(*pItData > 1) {
                        *pItData = 1;
                    }else if( *pItData < -1){
                        *pItData = -1;
                    }
                    pItData++, pItDevia++;
                }
                pItWeightDevia += itVar->size;
            }
            break;
        }
        itVar++;
    }
    return sCtx.success();
}

int COpenCLNetwork::toArchive(const SArchive& ar) {
    m_sSolveGraph.toArchive(ar);
    ar.visitString("optimizer", m_strOptimizer);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(COpenCLNetwork, COpenCLNetwork::__getClassKey())