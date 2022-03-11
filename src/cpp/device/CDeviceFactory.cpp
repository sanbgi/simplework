
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

    int getOpenclDevice(SDevice& spDevice) {
        static SDevice g_openclDevice = SObject::createObject("sw.device.OpenclDevice");
        spDevice = g_openclDevice;
        return sCtx.success();
    }

    int getDefaultDevice(SDevice& spDevice) {
        return getCpuDevice(spDevice);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CDeviceFactory, SDeviceFactory::__getClassKey())