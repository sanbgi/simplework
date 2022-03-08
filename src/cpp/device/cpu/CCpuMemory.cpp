
#include "../device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CCpuMemory");
class CCpuMemory : public CObject, public IDeviceMemory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PMemory* pMemory = CData<PMemory>(pData);
        if(pMemory == nullptr || pMemory->size < 1) {
            return sCtx.error("创建内存参数无效");
        }

        m_sMemory = *pMemory;
        m_sMemory.pByteArray = new unsigned char[m_sMemory.size];
        m_spTaker.take(m_sMemory.pByteArray, [](unsigned char* pBuffer){
            delete[] pBuffer;
        });
        if(pMemory->pByteArray != nullptr) {
            memcpy(m_sMemory.pByteArray, pMemory->pByteArray, pMemory->size);
        }
        return sCtx.success();
    }

private://IDeviceMemory
    int getDevice(SDevice& spDevice){
        return SDeviceFactory::getFactory()->getCpuDevice(spDevice);
    }

    int getMemory(PMemory& devcieMemory){
        if(!m_spTaker) {
            return sCtx.error();
        }
        devcieMemory = m_sMemory;
        return sCtx.success();
    }

    int setDeviceMemory(PMemory cpuDeviceMemory, int iOffset=0){
        if(!m_spTaker) {
            return sCtx.error();
        }
        if(cpuDeviceMemory.size + iOffset > m_sMemory.size) {
            return sCtx.error("设置内存超出了范围");
        }
        memcpy(m_sMemory.pByteArray + iOffset, cpuDeviceMemory.pByteArray, cpuDeviceMemory.size);
        return sCtx.success();
    }

    int getDeviceMemory(PMemory cpuDeviceMemory, int iOffset=0){
        if(!m_spTaker) {
            return sCtx.error();
        }
        if(cpuDeviceMemory.size + iOffset > m_sMemory.size) {
            return sCtx.error("设置内存超出了范围");
        }
        memcpy(cpuDeviceMemory.pByteArray, m_sMemory.pByteArray + iOffset , cpuDeviceMemory.size);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuMemory"; }

private:
    PMemory m_sMemory;
    CTaker<unsigned char*> m_spTaker;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CCpuMemory, CCpuMemory::__getClassKey())