
#include "../device.h"
#include <map>
#include <string>
#include <fstream> 
#include <iostream>
//#define CL_HPP_ENABLE_EXCEPTIONS
//#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include "cl/cl2.hpp"

using namespace sw;
using namespace std;

static SCtx sCtx("COpencl");

class COpenclMemory : public CObject, IKernelMemory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelMemory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PMemory* pMemory = CData<PMemory>(pData);
        if(pMemory == nullptr || pMemory->size < 1) {
            return sCtx.error("创建内存参数无效");
        }

        cl_int err;
        if(pMemory->data != nullptr) {
            m_sBuffer = cl::Buffer(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, (cl::size_type)pMemory->size, pMemory->data, &err);
        }else{
            m_sBuffer = cl::Buffer(CL_MEM_READ_WRITE, (cl::size_type)pMemory->size, nullptr, &err);
        }
        if( err != CL_SUCCESS ) {
            return sCtx.error("创建Opencl内存失败");
        }

        m_nSize = pMemory->size;
        return sCtx.success();
    }

private://IKernelMemory
    int getSize() {
        return m_nSize;
    }

    int getDevice(SDevice& spDevice){
        spDevice = SDevice::opencl();
        return sCtx.success();
    }

    void* getData(){
        return m_sBuffer.get();
    }

    int writeMemory(const SKernelMemory& spMemory) {
        if(spMemory.getPtr() == this) {
            return sCtx.success();
        }

        if(spMemory.size() != m_nSize) {
            return sCtx.error("不能写入大小不一样的内存");
        }

        SDevice spDevice = spMemory.device();
        if( spDevice.isCpu() ) {
            return writeMemory(m_nSize, spMemory.data());
        }else if( spDevice.isOpencl() ) {
            void* pSrc = spMemory.data();
            if( pSrc == m_sBuffer.get() ) {
                return sCtx.success();
            }
            //
            //  如果都是Opencl设备，数据拷贝是不是可以不通过内存中转，直接拷贝？这里面涉
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
    }

    int writeMemory(int nSize, void* pData, int iOffset=0){
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(((unsigned char*)pData), ((unsigned char*)pData)+nSize, m_sBuffer) != CL_SUCCESS ) {
            return sCtx.error("Opencl内存拷贝错误");
        }
        return sCtx.success();
    }

    int readMemory(int nSize, void* pData, int iOffset=0){
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        cl_int err;
        if( (err = cl::copy(m_sBuffer, ((unsigned char*)pData), ((unsigned char*)pData)+nSize )) != CL_SUCCESS ) {
            return sCtx.error("Opencl内存拷贝错误");
        }
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.OpenclMemory"; }

private:
    int m_nSize;
    cl::Buffer m_sBuffer;
};
SIMPLEWORK_FACTORY_AUTO_REGISTER(COpenclMemory, COpenclMemory::__getClassKey())

class COpenclDevice : public CObject, public IDevice{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        static bool s_initialized = false;
        static std::once_flag s_initializedFlag;
        std::call_once(s_initializedFlag, [&]{
            // Filter for a 2.0 platform and set it as the default
            std::vector<cl::Platform> platforms;
            cl::Platform::get(&platforms);
            cl::Platform plat;
            for (auto &p : platforms) {
                std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
                std::cout << "opencl ver:" << platver << "\n"; 
            }
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

        cl_int err = CL_SUCCESS;
        cl::Device device = cl::Device::getDefault(&err);
        if(err != CL_SUCCESS) {
            return sCtx.error("无法获得默认Opencl设备");
        }

        auto maxWorkItemSize = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&err);
        if(err != CL_SUCCESS) {
            return sCtx.error("Opencl设备最大工作项未知");
        }

        int nMaxRange = sizeof(m_pMaxRanges)/sizeof(m_pMaxRanges[0]);
        int nMaxDeviceRange = (int)maxWorkItemSize.size();
        m_nMaxRanges = nMaxDeviceRange > nMaxRange ? nMaxRange : nMaxDeviceRange;
        for(int i=0; i<m_nMaxRanges; i++) {
            m_pMaxRanges[i] = (int)maxWorkItemSize[i];
        }
        
        if( cl::CommandQueue::getDefault().get() == nullptr ) {
            return sCtx.error("创建默认的CommandQuere失败");
        }
        return sCtx.success();
    }

private://IDevice
    int createKernelMemory(SKernelMemory& spDeviceMemory, int nSize, void* pInitData = nullptr){
        spDeviceMemory = SObject::createObject("sw.device.OpenclMemory", CData<PMemory>({nSize, pInitData}));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建内存失败");
    }

    int createKernelMemory(SKernelMemory& spDeviceMemory, const SKernelMemory& spMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spMemory;
            return sCtx.success();
        }

        if(spDevice.isCpu() ) {
            return createKernelMemory(spDeviceMemory, spMemory.size(), spMemory.data());
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
                    PKernelVariable pArgs[], 
                    int nRanges = 0, 
                    int pRanges[]=nullptr) {
        
        //
        // 获取内核
        //
        cl::Kernel kernel;
        if( getKernel(kernelKey, kernel) != sCtx.success() ) {
            return sCtx.error("内核计算错误，找不到指定的内核");
        }

        //
        // 设置内核参数
        //
        PKernelVariable* pArg = pArgs;
        for(int i=0; i<nArgs; i++, pArg++) {
            kernel.setArg(i, pArg->size, pArg->data);
        }

        if(nRanges < 0 || nRanges > m_nMaxRanges ) {
            return sCtx.error("内核计算参数错误，nRnages不符合要求");
        }

        cl::Event event;
        cl::NDRange globalRange;
        switch(nRanges) {
            case 0:
                globalRange = cl::NDRange(1);
                break;

            case 1:
                globalRange = cl::NDRange(pRanges[0]);
                break;

            case 2:
                globalRange = cl::NDRange(pRanges[0],pRanges[1]);
                break;

            case 3:
                globalRange = cl::NDRange(pRanges[0],pRanges[1],pRanges[2]);
                break;

            default:
                return sCtx.error("内核计算范围不支持超过3个维度");
        }
        cl_int ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            globalRange,
            cl::NullRange,
            nullptr,
            nullptr//&event
        );
        
        if(ret != CL_SUCCESS) {
            return sCtx.error("OpenCL计算错误");
        }
        //if( (ret = event.wait()) != CL_SUCCESS ) {
        //    return sCtx.error("等待执行结果失败");
        //}
        return sCtx.success();
    }

private:
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

        string kernelName = kernelKey.runtimeKey;
        auto iProgramName = kernelName.rfind('.');
        if( iProgramName <= 0 && iProgramName >= kernelName.length() - 1) {
            return sCtx.error((string("无效的内核名，")+kernelName).c_str());
        }

        cl::Program clProgram;
        if( getProgram(kernelName.substr(0,iProgramName), clProgram) != sCtx.success() ) {
            return sCtx.error((string("内核创建错误，内核名：")+kernelKey.runtimeKey).c_str());
        }
        
        cl::Kernel kernel = cl::Kernel(clProgram, kernelName.substr(iProgramName+1).c_str());
        if(kernel.get() == nullptr) {
            return sCtx.error((string("创建内核错误，内核名：")+kernelKey.runtimeKey).c_str());
        }

        kernelFunc = kernel;
        sMapKernels[PRuntimeKey(kernelKey.runtimeKey).runtimeId] = kernel;
        sNamedKernels[kernelName] = kernel;
        return sCtx.success();
    }

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
        auto filename = "opencl/"+name.substr(0,ipos)+"/"+name.substr(ipos+1)+".cl";
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
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.opencl.OpenclDevice"; }

private:
    int m_nMaxRanges;
    int m_pMaxRanges[3];
};
SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(COpenclDevice, COpenclDevice::__getClassKey())