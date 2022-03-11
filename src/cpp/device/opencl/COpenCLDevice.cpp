
#include "../device.h"
#include <map>
#include <string>
#include <fstream> 
#include <iostream>
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include "cl/cl2.hpp"

using namespace sw;
using namespace std;

static SCtx sCtx("COpencl");

class COpenclMemory : public CObject, IDeviceMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
        //SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PMemory* pMemory = CData<PMemory>(pData);
        if(pMemory == nullptr || pMemory->size < 1) {
            return sCtx.error("创建内存参数无效");
        }

        m_nSize = pMemory->size;
        cl_int err;
        m_sBuffer = cl::Buffer(CL_MEM_READ_WRITE, (cl::size_type)pMemory->size, nullptr, &err);
        if( err != CL_SUCCESS ) {
            return sCtx.error("创建Opencl内存失败");
        }

        if(pMemory->data != nullptr) {
            if( setMemory(*pMemory) != sCtx.success() ) {
                return sCtx.error("拷贝Opencl内存失败");
            }
        }
        return sCtx.success();
    }

protected://IArchivable
    int getClassVer() { return 220308; }
    const char* getClassKey() { return COpenclMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("size", m_nSize);
        //TODO
        return sCtx.success();
    }

private://IDeviceMemory
    int getSize() {
        return m_nSize;
    }

    int getDevice(SDevice& spDevice){
        return SDeviceFactory::getFactory()->getOpenclDevice(spDevice);
    }

    int getMemoryInDevice(const SDevice& spDevice, PMemory& deviceMemory){
        if( spDevice.getPtr() != SDevice::opencl().getPtr() ) {
            return sCtx.error("无法获取非Opencl设备内存");
        }
        deviceMemory.size = m_nSize;
        deviceMemory.data = m_sBuffer.get();
        return sCtx.success();
    }

    int setMemory(const PMemory& cpuMemory, int iOffset=0){
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if(cpuMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(cpuMemory.pByteArray, cpuMemory.pByteArray+cpuMemory.size, m_sBuffer) != CL_SUCCESS ) {
            return sCtx.error("Opencl内存拷贝错误");
        }
        return sCtx.success();
    }

    int getMemory(const PMemory& cpuMemory, int iOffset=0){
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if(cpuMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(m_sBuffer, cpuMemory.pByteArray, cpuMemory.pByteArray+cpuMemory.size ) != CL_SUCCESS ) {
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

private://IDevice
    int createMemory(const PMemory& cpuMemory, SDeviceMemory& spDeviceMemory){
        spDeviceMemory = SObject::createObject("sw.device.OpenclMemory", CData<PMemory>(cpuMemory));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建内存失败");
    }

    int createMemory(const SDeviceMemory& spSrcMemory, SDeviceMemory& spDeviceMemory){
        SDevice spDevice = spSrcMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spSrcMemory;
            return sCtx.success();
        }

        int size = spSrcMemory.size();
        CTaker<char*> spTaker(new char[size], [](char* pMemory){
            delete[] pMemory;
        });

        PMemory sMemory = {size, spTaker};
        if( !spSrcMemory || spSrcMemory->getMemory(sMemory) != sCtx.success() ) {
            return sCtx.error("创建内存所对应的原始内存无效");
        }
        return createMemory(sMemory, spDeviceMemory);
    }

    int runKernel(const PKernalKey& kernelKey, int nArgs, PMemory pArgs[], int nRanges=0, int pRanges[]=nullptr, SDeviceEvent* pEvent=nullptr) {
        cl::Kernel kernel;
        if( getKernel(kernelKey, kernel) != sCtx.success() ) {
            return sCtx.error("内核计算错误，找不到指定的内核");
        }

        PMemory* pArg = pArgs;
        for(int i=0; i<nArgs; i++, pArg++) {
            kernel.setArg(i,pArg->size, pArg->data);
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
            &event
        );
        if(ret != CL_SUCCESS) {
            return sCtx.error("OpenCL计算错误");
        }
        event.wait();
        return sCtx.success();
    }

private:
    static int getKernel(const PKernalKey& kernelKey, cl::Kernel& kernelFunc) {
        static std::map<PID,cl::Kernel> sMapKernels;
        static std::map<string,cl::Program> sMapPrograms;
        if(kernelKey.pKernalId && *kernelKey.pKernalId > 0) {
            auto it = sMapKernels.find(*kernelKey.pKernalId);
            if( it != sMapKernels.end() ) {
                kernelFunc = it->second;
                return sCtx.success();
            }
        }

        if( kernelKey.szKernalName == nullptr || kernelKey.szProgramName == nullptr) {
            return sCtx.error("内核参数错误");
        }

        cl::Program clProgram;
        string strProgramName = kernelKey.szProgramName;
        auto itProgram = sMapPrograms.find(strProgramName);
        if( itProgram == sMapPrograms.end() ) {
            if( getProgram(strProgramName, clProgram) != sCtx.success() ) {
                return sCtx.error((string("内核创建错误，内核名：")+strProgramName).c_str());
            }
            sMapPrograms[strProgramName] = clProgram;
        }else{
            clProgram = itProgram->second;
        }

        cl::Kernel kernel = cl::Kernel(clProgram, kernelKey.szKernalName);
        if(kernel.get() == nullptr) {
            string errMsg = string("创建内核错误，程序名：")+strProgramName+", 内核名:"+kernelKey.szKernalName;
            return sCtx.error(errMsg.c_str());
        }
        
        string keyName = string(kernelKey.szProgramName)+"."+kernelKey.szKernalName;
        PRuntimeKey rKey(keyName.c_str());
        sMapKernels[rKey.runtimeId] = kernel;
        if(kernelKey.pKernalId != nullptr && *kernelKey.pKernalId != rKey.runtimeId) {
            *kernelKey.pKernalId = rKey.runtimeId;
        }
        kernelFunc = kernel;
        return sCtx.success();
    }

    static int getProgram(string name, cl::Program& program) {
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

public://Factory
    static const char* __getClassKey() { return "sw.device.OpenclDevice"; }
};
SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(COpenclDevice, COpenclDevice::__getClassKey())