
#include "../device.h"
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
        m_sBuffer = cl::Buffer(CL_MEM_READ_WRITE, pMemory->size, nullptr, &err);
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
        return SDeviceFactory::getFactory()->getCpuDevice(spDevice);
    }

    int getMemoryInDevice(const SDevice& spDevice, PMemory& deviceMemory){
        if( spDevice != SDevice::opencl() ) {
            return sCtx.error("无法获取非Opencl设备内存");
        }
        deviceMemory.size = m_nSize;
        deviceMemory.data = &m_sBuffer;
        return sCtx.success();
    }

    int setMemory(PMemory cpuMemory, int iOffset=0){
        //TODO
        //if(!m_sBuffer) {
        //    return sCtx.error("设备内存无效");
        //}
        if(cpuMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        if( cl::copy(cpuMemory.pByteArray, cpuMemory.pByteArray+cpuMemory.size, m_sBuffer) != CL_SUCCESS ) {
            return sCtx.error("Opencl内存拷贝错误");
        }
        return sCtx.success();
    }

    int getMemory(PMemory cpuMemory, int iOffset=0){
        //TODO
        //if(!m_sBuffer) {
        //    return sCtx.error("设备内存无效");
        //}
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

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(COpenclMemory, COpenclMemory::__getClassKey())

class COpenclDevice : public CObject, public IDevice{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
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

        SDeviceMemory spMemory;
        return createMemory(sMemory, spMemory);
    }

    int runKernel(const PKernalKey& kernelKey, int nArgs, PMemory pArgs[], int nRanges=0, int pRanges[]=nullptr, SDeviceEvent* pEvent=nullptr) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.OpenclDevice"; }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(COpenclDevice, COpenclDevice::__getClassKey())