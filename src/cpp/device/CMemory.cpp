
#include "device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CMemory");
class CMemory : public CObject, IMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IMemory)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PMemory* pMemory = CData<PMemory>(pData);
        if(pMemory == nullptr || pMemory->size < 1) {
            return sCtx.error("创建内存参数无效");
        }
        return SDevice::defaultDevice()->createMemory(*pMemory, m_spMemory);
    }

protected://IArchivable
    int getClassVer() { return 220308; }
    const char* getClassKey() { return SMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        if(!ar->isReading()) {
            PMemory sMemory;
            if( getMemoryInDevice(SDevice::cpu(), sMemory) != sCtx.success() ) {
                return sCtx.error("内存无法保存到CPU");
            }
        }
        return ar.arObject("data", m_spMemory);
    }


private://IMemory
    int getDevice(SDevice& spDevice){
        return m_spMemory->getDevice(spDevice);
    }

    int getMemoryInDevice(const SDevice& spDevice, PMemory& deviceMemory){
        if( m_spMemory->getMemoryInDevice(spDevice, deviceMemory) != sCtx.success() ) {
            if( spDevice->createMemory(m_spMemory, m_spMemory) != sCtx.success() ) {
                return sCtx.error("创建设备内存异常");
            }
        }
        return m_spMemory->getMemoryInDevice(spDevice, deviceMemory);
    }

    int setMemory(PMemory cpuMemory, int iOffset=0){
        return m_spMemory->setMemory(cpuMemory, iOffset);
    }

    int getMemory(PMemory cpuMemory, int iOffset=0){
        return m_spMemory->getMemory(cpuMemory, iOffset);
    }

private:
    SMemory m_spMemory;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CMemory, SMemory::__getClassKey())