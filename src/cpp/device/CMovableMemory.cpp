
#include "device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CMovableMemory");
class CMovableMemory : public CObject, IMovableMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IMovableMemory)
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
    const char* getClassKey() { return SMovableMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        if(!ar->isReading()) {
            if( getDataInDevice(SDevice::cpuDevice()) != sCtx.success() ) {
                return sCtx.error("内存无法保存到CPU");
            }
        }
        return ar.arObject("data", m_spMemory);
    }


private://IMovableMemory
    int getDataInDevice(const SDevice& spDevice, PMemory* pDeviceMemory=nullptr){
        if( spDevice->createMemory(m_spMemory, m_spMemory) != sCtx.success() ) {
            return sCtx.error("创建设备内存异常");
        }
        if(pDeviceMemory!= nullptr) {
            return m_spMemory->getMemory(*pDeviceMemory);
        }
        return sCtx.success();
    }

    int setMemory(PMemory cpuMemory, int iOffset=0){
        return m_spMemory->setDeviceMemory(cpuMemory,iOffset);
    }

    int getMemory(PMemory cpuMemory, int iOffset=0){
        return m_spMemory->getDeviceMemory(cpuMemory,iOffset);
    }

private:
    SDeviceMemory m_spMemory;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CMovableMemory, SMovableMemory::__getClassKey())