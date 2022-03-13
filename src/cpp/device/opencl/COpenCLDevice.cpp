
#include "../device.h"
#include <map>
#include <string>
#include <fstream> 
#include <iostream>
//#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include "cl/cl2.hpp"

using namespace sw;
using namespace std;

static SCtx sCtx("COpencl");

class COpenclMemory : public CObject, IDeviceMemory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
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

private://IDeviceMemory
    int getSize() {
        return m_nSize;
    }

    int getDevice(SDevice& spDevice){
        return SDeviceFactory::getFactory()->getOpenclDevice(spDevice);
    }

    int getMemory(const SDevice& spDevice, PMemory& deviceMemory){
        if( spDevice.getPtr() != SDevice::opencl().getPtr() ) {
            return sCtx.error("无法获取非Opencl设备内存");
        }
        deviceMemory.size = m_nSize;
        deviceMemory.data = m_sBuffer.get();
        return sCtx.success();
    }

    int writeMemory(const PMemory& cpuMemory, int iOffset=0){
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || cpuMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(cpuMemory.pByteArray, cpuMemory.pByteArray+cpuMemory.size, m_sBuffer) != CL_SUCCESS ) {
            return sCtx.error("Opencl内存拷贝错误");
        }
        return sCtx.success();
    }

    int readMemory(const PMemory& cpuMemory, int iOffset=0){
        if(m_sBuffer.get() == nullptr) {
            return sCtx.error("设备内存无效");
        }
        if( iOffset != 0 || cpuMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        cl_int err;
        if( (err = cl::copy(m_sBuffer, cpuMemory.pByteArray, cpuMemory.pByteArray+cpuMemory.size )) != CL_SUCCESS ) {
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
        int nMaxDeviceRange = maxWorkItemSize.size();
        m_nMaxRanges = nMaxDeviceRange > nMaxRange ? nMaxRange : nMaxDeviceRange;
        for(int i=0; i<m_nMaxRanges; i++) {
            m_pMaxRanges[i] = maxWorkItemSize[i] * 100000;
        }
        
        if( cl::CommandQueue::getDefault().get() == nullptr ) {
            return sCtx.error("创建默认的CommandQuere失败");
        }
        return sCtx.success();
    }

private://IDevice
    int createMemory(const PMemory& cpuMemory, SDeviceMemory& spDeviceMemory){
        spDeviceMemory = SObject::createObject("sw.device.OpenclMemory", CData<PMemory>(cpuMemory));
        return spDeviceMemory ? sCtx.success() : sCtx.error("创建内存失败");
    }

    int createMemory(const SDeviceMemory& spMemory, SDeviceMemory& spDeviceMemory){
        SDevice spDevice = spMemory.device();
        if(spDevice.getPtr() == this) {
            spDeviceMemory = spMemory;
            return sCtx.success();
        }

        if(spDevice.getPtr() == SDevice::cpu().getPtr()) {
            PMemory sMemory;
            if( spMemory->getMemory(spDevice, sMemory) != sCtx.success() ) {
                return sCtx.error("无法获取内存指针");
            }
            return createMemory(sMemory, spDeviceMemory);
        }

        //如果不是CPU内存，则需要内存拷贝到CPU内存，作为中转
        int size = spMemory.size();
        CTaker<char*> spTaker(new char[size], [](char* pMemory){
            delete[] pMemory;
        });

        PMemory sMemory = {size, spTaker};
        if( !spMemory || spMemory->readMemory(sMemory) != sCtx.success() ) {
            return sCtx.error("创建内存所对应的原始内存无效");
        }
        return createMemory(sMemory, spDeviceMemory);
    }

    int runKernel(  const PKernalKey& kernelKey, 
                    int nArgs, 
                    PKernalVariable pArgs[], 
                    int nRanges = 0, 
                    int pRanges[]=nullptr) {
        cl::Kernel kernel;
        if( getKernel(kernelKey, kernel) != sCtx.success() ) {
            return sCtx.error("内核计算错误，找不到指定的内核");
        }

        PKernalVariable* pArg = pArgs;
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
            &event
        );
        
        /*
        //如果内核支持不支持超过范围的RANGE，则可以启用下面代码来拆分指令（目前看起来必要性不高)
        cl_int ret = CL_SUCCESS;
        cl::Event event;
        if(nRanges == 0) {
            ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(1),
                cl::NullRange,
                nullptr,
                &event
            );
        }else{
            int iOffset[3] = {0, 0, 0};
            int nWorkSize[3];
            int iRuningLayer=nRanges-1;
            while(iRuningLayer>=0) {
                int sum = 0;
                for(int i=0; i<nRanges; i++) {
                    if( pRanges[i] - iOffset[i] > m_pMaxRanges[i]) {
                        nWorkSize[i] = m_pMaxRanges[i];
                    }else{
                        nWorkSize[i] = pRanges[i] - iOffset[i];
                    }
                    sum += nWorkSize[i];
                }
                if(sum == 0) {
                    break;
                }

                if(nRanges == 1) {
                    ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
                        kernel,
                        cl::NDRange(iOffset[0]),
                        cl::NDRange(nWorkSize[0]),
                        cl::NullRange,
                        nullptr,
                        &event
                    );
                }else if(nRanges == 2) {
                    ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
                        kernel,
                        cl::NDRange(iOffset[0], iOffset[1]),
                        cl::NDRange(nWorkSize[0], nWorkSize[1]),
                        cl::NullRange,
                        nullptr,
                        &event
                    );
                }else if(nRanges == 3) {
                    ret = cl::CommandQueue::getDefault().enqueueNDRangeKernel(
                        kernel,
                        cl::NDRange(iOffset[0], iOffset[1], iOffset[2]),
                        cl::NDRange(nWorkSize[0], nWorkSize[1], nWorkSize[2]),
                        cl::NullRange,
                        nullptr,
                        &event
                    );
                }
                if(ret != CL_SUCCESS) {
                    break;
                }
                
                iRuningLayer = nRanges-1;
                while(iRuningLayer>=0){
                    iOffset[iRuningLayer] += nWorkSize[iRuningLayer];
                    if(iOffset[iRuningLayer] < pRanges[iRuningLayer]) {
                        break;
                    }
                    iOffset[iRuningLayer--] = 0;
                }
            }
        }*/
        
        if(ret != CL_SUCCESS) {
            return sCtx.error("OpenCL计算错误");
        }
        event.wait();
        return sCtx.success();
    }

private:
    static int getKernel(const PKernalKey& kernelKey, cl::Kernel& kernelFunc) {
        static std::map<PID,cl::Kernel> sMapKernels;
        static std::map<string,cl::Kernel> sNamedKernels;
        if(kernelKey.pKernalId) {
            if( *kernelKey.pKernalId > 0 ) {
                auto it = sMapKernels.find(*kernelKey.pKernalId);
                if( it != sMapKernels.end() ) {
                    kernelFunc = it->second;
                    return sCtx.success();
                }
                return sCtx.error("发现无效的KernalID");
            }
        }

        if(kernelKey.szKernalName == nullptr) {
            return sCtx.error("内核参数错误");
        }

        auto it = sNamedKernels.find(kernelKey.szKernalName);
        if( it != sNamedKernels.end() ) {
            if(kernelKey.pKernalId != nullptr ) {
                PRuntimeKey rKey(kernelKey.szKernalName);
                sMapKernels[rKey.runtimeId] = it->second;
                *kernelKey.pKernalId = rKey.runtimeId;
            }
            kernelFunc = it->second;
            return sCtx.success();
        }

        string kernalName = kernelKey.szKernalName;
        auto iProgramName = kernalName.rfind('.');
        if( iProgramName <= 0 && iProgramName >= kernalName.length() - 1) {
            return sCtx.error((string("无效的内核名，")+kernalName).c_str());
        }

        cl::Program clProgram;
        if( getProgram(kernalName.substr(0,iProgramName), clProgram) != sCtx.success() ) {
            return sCtx.error((string("内核创建错误，内核名：")+kernelKey.szKernalName).c_str());
        }
        
        cl::Kernel kernel = cl::Kernel(clProgram, kernalName.substr(iProgramName+1).c_str());
        if(kernel.get() == nullptr) {
            return sCtx.error((string("创建内核错误，内核名：")+kernelKey.szKernalName).c_str());
        }
        
        PRuntimeKey rKey(kernelKey.szKernalName);
        sMapKernels[rKey.runtimeId] = kernel;
        if(kernelKey.pKernalId != nullptr ) {
            *kernelKey.pKernalId = rKey.runtimeId;
        }

        kernelFunc = kernel;
        sNamedKernels[kernalName] = kernel;
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
    static const char* __getClassKey() { return "sw.device.OpenclDevice"; }

private:
    int m_nMaxRanges;
    int m_pMaxRanges[3];
};
SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(COpenclDevice, COpenclDevice::__getClassKey())