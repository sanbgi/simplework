
#include "../device.h"
#include <map>
#include <string>
#include <fstream> 
#include <iostream>
#include <cuda.h>


using namespace sw;
using namespace std;

static SCtx sCtx("CCuda");

class CCudaMemory : public CObject, IDeviceMemory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        return sCtx.success();
    }

private://IDeviceMemory
    int getSize() {
        return m_nSize;
    }

    int getDevice(SDevice& spDevice){
        spDevice = SDevice::cuda();
        return sCtx.success();
    }

    void* getData(const SDevice& spDevice){
        if( spDevice.getPtr() != SDevice::cuda().getPtr() ) {
            return nullptr;
        }
        //return m_sBuffer.get();
        return nullptr;
    }

    int writeMemory(const SDeviceMemory& spMemory) {
        if(spMemory.getPtr() == this) {
            return sCtx.success();
        }

        /*
        if(spMemory.size() != m_nSize) {
            return sCtx.error("不能写入大小不一样的内存");
        }

        SDevice spDevice = spMemory.device();
        if( spDevice.getPtr() == SDevice::cpu().getPtr() ) {
            return writeMemory(m_nSize, spMemory.data(spDevice));
        }else if( spDevice.getPtr() == SDevice::Cuda().getPtr() ) {
            void* pSrc = spMemory.data(spDevice);
            if( pSrc == m_sBuffer.get() ) {
                return sCtx.success();
            }
            //
            //  如果都是Cuda设备，数据拷贝是不是可以不通过内存中转，直接拷贝？这里面涉
            //  及到需要内核函数执行，依赖性较强
            //
            return spDevice.memoryCopy(m_sBuffer.get(), 0, pSrc, 0, m_nSize);
        }
        
        CTaker<char*> spTaker(new char[m_nSize], [](char* pMemory){
            delete[] pMemory;
        });
        if( spMemory->readMemory(m_nSize, spTaker) != sCtx.success() ){
            return sCtx.error("读取数据源数据异常");
        }
        return writeMemory(m_nSize, spTaker);
        */
        return sCtx.success();
    }

    int writeMemory(int nSize, void* pData, int iOffset=0){
        /*
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(((unsigned char*)pData), ((unsigned char*)pData)+nSize, m_sBuffer) != CL_SUCCESS ) {
            return sCtx.error("Cuda内存拷贝错误");
        }
        */
        return sCtx.success();
    }

    int readMemory(int nSize, void* pData, int iOffset=0){
        /*
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        cl_int err;
        if( (err = cl::copy(m_sBuffer, ((unsigned char*)pData), ((unsigned char*)pData)+nSize )) != CL_SUCCESS ) {
            return sCtx.error("Cuda内存拷贝错误");
        }*/
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CudaMemory"; }

private:
    int m_nSize;
    //cl::Buffer m_sBuffer;
};
SIMPLEWORK_FACTORY_AUTO_REGISTER(CCudaMemory, CCudaMemory::__getClassKey())

class CCudaDevice : public CObject, public IDevice{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        if( cuInit(0) != CUDA_SUCCESS ) {
            return sCtx.error("CUDA 初始化失败");
        } 
        
        return sCtx.success();
    }

private://IDevice
    int createKernelMemory(SDeviceMemory& spDeviceMemory, int nSize, void* pInitData = nullptr){
        spDeviceMemory = SObject::createObject("sw.device.CudaMemory", CData<PMemory>({nSize, pInitData}));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建内存失败");
    }

    int createKernelMemory(SDeviceMemory& spDeviceMemory, const SDeviceMemory& spMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spMemory;
            return sCtx.success();
        }

        if(spDevice.getPtr() == SDevice::cpu().getPtr()) {
            void* pData = spMemory.data(spDevice);
            if( pData == nullptr ) {
                return sCtx.error("无法获取内存指针");
            }
            return createKernelMemory(spDeviceMemory, spMemory.size(), pData);
        }

        //如果不是CPU内存，则需要内存拷贝到CPU内存，作为中转
        int size = spMemory.size();
        CTaker<char*> spTaker(new char[size], [](char* pMemory){
            delete[] pMemory;
        });

        if( !spMemory || spMemory->readMemory(size, spTaker) != sCtx.success() ) {
            return sCtx.error("创建内存所对应的原始内存无效");
        }
        return createKernelMemory(spDeviceMemory, size, spTaker);
    }

    int runKernel(  const PRuntimeKey& kernelKey, 
                    int nArgs, 
                    PKernalVariable pArgs[], 
                    int nRanges = 0, 
                    int pRanges[]=nullptr) {
        return sCtx.success();
    }

private:
    /*
    static int getKernel(const PRuntimeKey& kernelKey, cl::Kernel& kernelFunc) {

        static std::map<PID,cl::Kernel> sMapKernels;
        static std::map<string,cl::Kernel> sNamedKernels;

        auto it = sMapKernels.find(kernelKey.runtimeId);
        if( it != sMapKernels.end() ) {
            kernelFunc = it->second;
            return sCtx.success();
        }

        if(kernelKey.runtimeKey == nullptr) {
            return sCtx.error("内核参数错误");
        }

        auto itNamedKernel = sNamedKernels.find(kernelKey.runtimeKey);
        if( itNamedKernel != sNamedKernels.end() ) {
            sMapKernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = itNamedKernel->second;
            kernelFunc = itNamedKernel->second;
            return sCtx.success();
        }

        string kernalName = kernelKey.runtimeKey;
        auto iProgramName = kernalName.rfind('.');
        if( iProgramName <= 0 && iProgramName >= kernalName.length() - 1) {
            return sCtx.error((string("无效的内核名，")+kernalName).c_str());
        }

        //cl::Program clProgram;
        //if( getProgram(kernalName.substr(0,iProgramName), clProgram) != sCtx.success() ) {
        //    return sCtx.error((string("内核创建错误，内核名：")+kernelKey.runtimeKey).c_str());
        //}
        
        //cl::Kernel kernel = cl::Kernel(clProgram, kernalName.substr(iProgramName+1).c_str());
        //if(kernel.get() == nullptr) {
        //    return sCtx.error((string("创建内核错误，内核名：")+kernelKey.runtimeKey).c_str());
        //}

        //kernelFunc = kernel;
        //sMapKernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = kernel;
        //sNamedKernels[kernalName] = kernel;
        return sCtx.success();
    }
    */

    /*
    static int getProgram(string name, cl::Program& program) {
        
        static std::map<string,cl::Program> sMapPrograms;
        auto itProgram = sMapPrograms.find(name);
        if( itProgram != sMapPrograms.end() ) {
            program = itProgram->second;
            return sCtx.success();
        }

        auto ipos = name.rfind(".");
        if( ipos == string::npos || ipos < 1 || ipos >= name.length() - 1) {
            return sCtx.error(("内核程序名错误, 名字："+name).c_str());
        }
        auto filename = "Cuda/"+name.substr(0,ipos)+"/"+name.substr(ipos+1)+".cl";
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

        cl::Program clProgram(data.data());
        try {
            cl_int ret = clProgram.build("-cl-std=CL2.0");
            if( ret != CL_SUCCESS) {
                return sCtx.error(("编译内核文件错误，文件名："+filename).c_str());
            }
        }
        catch (...) {
            // Print build info for all devices
            cl_int buildErr = CL_SUCCESS;
            auto buildInfo = clProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
            for (auto &pair : buildInfo) {
                std::cerr << pair.second << std::endl << std::endl;
            }
            return sCtx.error(("编译内核文件错误，文件名："+filename).c_str());
        }
        program = clProgram;
        sMapPrograms[name] = program;
        
        return sCtx.success();
    }*/

public://Factory
    static const char* __getClassKey() { return "sw.device.cuda.CudaDevice"; }

};
SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCudaDevice, CCudaDevice::__getClassKey())