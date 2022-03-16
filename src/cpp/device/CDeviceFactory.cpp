
#include "device.h"
#include <map>
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CDeviceFactory");
class CDeviceFactory : public CObject, public IDeviceFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    int getAvaiableDevices(const char* szName, IVisitor<const SDevice&>& cbVisitor) {
        return sCtx.success();
    }

    int getDevice(const char* szName, SDevice& spDevice) {
        static PMemory sInitMemory;
        static std::map<string,SDevice> sDeviceMap = {
            { "cpu", SObject::createObject("sw.device.CpuDevice", CData<PMemory>(sInitMemory)) },
            //{ "cuda", SObject::createObject("sw.device.cuda.CudaDevice") },
            { "opencl", SObject::createObject("sw.device.opencl.OpenclDevice", CData<PMemory>(sInitMemory)) },
        };
        auto it = sDeviceMap.find(szName);
        if(it != sDeviceMap.end()) {
            spDevice = it->second;
            return sCtx.success();
        }
        return sCtx.error((string("找不到对应的设备:")+szName).c_str());
    }

    int getCpuDevice(SDevice& spDevice) {
        static SDevice g_cpuDevice = SObject::createObject("sw.device.CpuDevice");
        spDevice = g_cpuDevice;
        return sCtx.success();
    }

    int getCudaDevice(SDevice& spDevice){
        static PMemory g_initMemory;
        static SDevice g_cudaDevice = SObject::createObject("sw.device.cuda.CudaDevice", CData<PMemory>(g_initMemory));
        spDevice = g_cudaDevice;
        return sCtx.success();
    }

    int getOpenclDevice(SDevice& spDevice) {
        static PMemory g_initMemory;
        static SDevice g_openclDevice = SObject::createObject("sw.device.opencl.OpenclDevice", CData<PMemory>(g_initMemory));
        spDevice = g_openclDevice;
        return sCtx.success();
    }

    int getDefaultKernelDevic(SDevice& spDevice) {
        if(!m_spDefaultKernelDevice) {
            m_spDefaultKernelDevice = SDevice::cpu();
        }
        spDevice = m_spDefaultKernelDevice;
        return sCtx.success();
    }

    int setDefaultKernelDevic(const SDevice& spDevice) {
        m_spDefaultKernelDevice = spDevice;
        return sCtx.success();
    }

    int getDefaultHostDevic(SDevice& spDevice) {
        if(!m_spDefaultHostDevice) {
            m_spDefaultHostDevice = SDevice::cpu();
        }
        spDevice = m_spDefaultHostDevice;
        return sCtx.success();
    }

    int setDefaultHostDevice(const SDevice& spDevice) {
        m_spDefaultHostDevice = spDevice;
        return sCtx.success();
    }

    SDevice m_spDefaultKernelDevice;
    SDevice m_spDefaultHostDevice;
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CDeviceFactory, SDeviceFactory::__getClassKey())