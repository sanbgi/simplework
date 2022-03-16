
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
        SDevice spDevice = pDeviceMemory->pDevice == nullptr ? SDevice::defaultHostDevice() : *pDeviceMemory->pDevice;
        if(pDeviceMemory->pKernelMemory) {
            SDeviceMemory spKernelMemory = *(pDeviceMemory->pKernelMemory);
            if( !spKernelMemory || spKernelMemory->getKernelMemory(spKernelMemory) != sCtx.success() ) {
                return sCtx.error("未知异常，按理说，所有内存对象必须返回一个内核内存对象");
            }
            return spDevice->createKernelMemory(m_spMemory, spKernelMemory);
        }
        return spDevice->createKernelMemory(m_spMemory, pDeviceMemory->size, pDeviceMemory->data);
    }

protected://IArchivable
    int getClassVer() { return 220308; }
    const char* getClassKey() { return SDeviceMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        if(!ar->isReading()) {
            m_spMemory = m_spMemory.toDevice(SDevice::cpu());
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

    int getKernelMemory(SDeviceMemory& spKernelMemory){
        spKernelMemory = m_spMemory;
        return sCtx.success();
    }

    int toDevice(const SDevice& spDevice, SDeviceMemory& spMemory) {
        if( spDevice->createKernelMemory(m_spMemory, m_spMemory) ) {
            return sCtx.error("设备转化异常");
        }
        spMemory.setPtr(this);
        return sCtx.success();
    }

    int writeMemory(const SDeviceMemory& spMemory){
        return m_spMemory->writeMemory(spMemory);
    }

    int writeMemory(int nSize, void* pData, int iOffset=0){
        return m_spMemory->writeMemory(nSize, pData, iOffset);
    }

    int readMemory(int nSize, void* pData, int iOffset=0){
        return m_spMemory->readMemory(nSize, pData, iOffset);
    }

private:
    SDeviceMemory m_spMemory;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDevicMemory, SDeviceMemory::__getClassKey())