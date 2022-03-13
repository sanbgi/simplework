
#include "device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CMemory");
class CMemory : public CObject, IDeviceMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PDeviceMemory* pDeviceMemory = CData<PDeviceMemory>(pData);
        if( pDeviceMemory == nullptr ) {
            const PMemory* pMemory = CData<PMemory>(pData);
            if(pMemory == nullptr || pMemory->size < 1) {
                return sCtx.error("创建内存参数无效");
            }
            return SDevice::cpu()->createMemory(*pMemory, m_spMemory);
        }
        return pDeviceMemory->spDevice->createMemory(pDeviceMemory->cpuMemory, m_spMemory);
    }

protected://IArchivable
    int getClassVer() { return 220308; }
    const char* getClassKey() { return SDeviceMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        if(!ar->isReading()) {
            PMemory sMemory;
            if( getMemoryInDevice(SDevice::cpu(), sMemory) != sCtx.success() ) {
                return sCtx.error("内存无法保存到CPU");
            }
        }
        return ar.arObject("data", m_spMemory);
    }


private://IDeviceMemory
    int getSize() {
        IDeviceMemory* pMemory = m_spMemory.getPtr();
        return pMemory != nullptr ? pMemory->getSize() : 0;
    }

    int getDevice(SDevice& spDevice){
        return m_spMemory->getDevice(spDevice);
    }

    int getMemoryInDevice(const SDevice& spDevice, PMemory& deviceMemory){
        SDevice spInDevice = m_spMemory.device();
        if( spInDevice.getPtr() != spDevice.getPtr() ) {
            SDeviceMemory toMemory;
            if( spDevice->createMemory(m_spMemory, toMemory) != sCtx.success()) {
                return sCtx.error("创建设备内存异常");
            }
            m_spMemory = toMemory;
        }
        return m_spMemory->getMemoryInDevice(spDevice,deviceMemory);
    }

    int setCpuMemory(const PMemory& cpuMemory, int iOffset=0){
        return m_spMemory->setCpuMemory(cpuMemory, iOffset);
    }

    int getCpuMemory(const PMemory& cpuMemory, int iOffset=0){
        return m_spMemory->getCpuMemory(cpuMemory, iOffset);
    }

private:
    SDeviceMemory m_spMemory;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CMemory, SDeviceMemory::__getClassKey())