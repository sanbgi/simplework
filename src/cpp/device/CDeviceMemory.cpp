
#include "device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CDevicMemory");
class CDevicMemory : public CObject, IDeviceMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PDeviceMemory* pDeviceMemory = CData<PDeviceMemory>(pData);
        if( pDeviceMemory == nullptr ){
            return sCtx.error("创建内存参数无效");
        }
        if( pDeviceMemory->pDevice == nullptr ) {
            return SDevice::cpu()->createKernelMemory(m_spMemory, pDeviceMemory->size, pDeviceMemory->data);
        }
        return pDeviceMemory->pDevice->createKernelMemory(m_spMemory, pDeviceMemory->size, pDeviceMemory->data);
    }

protected://IArchivable
    int getClassVer() { return 220308; }
    const char* getClassKey() { return SDeviceMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        if(!ar->isReading()) {
            //转化为CPU内存
            getData(SDevice::cpu());
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

    void* getData(const SDevice& spDevice){
        SDevice spInDevice = m_spMemory.device();
        if( spInDevice.getPtr() != spDevice.getPtr() ) {
            SDeviceMemory toMemory;
            if( spDevice->createKernelMemory(toMemory, m_spMemory) != sCtx.success()) {
                sCtx.error("创建设备内存异常");
                return nullptr;
            }
            m_spMemory = toMemory;
        }
        return m_spMemory->getData(spDevice);
    }

    int writeMemory(const PMemory& cpuMemory, int iOffset=0){
        return m_spMemory->writeMemory(cpuMemory, iOffset);
    }

    int readMemory(const PMemory& cpuMemory, int iOffset=0){
        return m_spMemory->readMemory(cpuMemory, iOffset);
    }

private:
    SDeviceMemory m_spMemory;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDevicMemory, SDeviceMemory::__getClassKey())