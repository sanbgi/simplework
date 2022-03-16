
#include "device.h"

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
        if(!m_spDefaultDevice) {
            m_spDefaultDevice = SDevice::cpu();
        }
        spDevice = m_spDefaultDevice;
        return sCtx.success();
    }

    int setDefaultKernelDevic(const SDevice& spDevice) {
        m_spDefaultDevice = spDevice;
        return sCtx.success();
    }

    SDevice m_spDefaultDevice;
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CDeviceFactory, SDeviceFactory::__getClassKey())