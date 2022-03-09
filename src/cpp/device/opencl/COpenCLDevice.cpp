
#include "../device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("COpenCLDevice");
class COpenCLDevice : public CObject, public IDevice{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDevice)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IDevice
    int createMemory(const PMemory& cpuMemory, SMemory& spDeviceMemory){
        return sCtx.success();
    }

    int createMemory(const SMemory& spSrcMemory, SMemory& spDeviceMemory){
        return sCtx.success();
    }

    int runKernel(const PRuntimeKey& opKey, int nArgs, PMemory pArgs[], int nRanges=0, int pRanges[]=nullptr, SDeviceEvent* pEvent=nullptr) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.OpenCLDevice"; }
};


SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(COpenCLDevice, COpenCLDevice::__getClassKey())