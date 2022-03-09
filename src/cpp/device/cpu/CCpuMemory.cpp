
#include "../device.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CCpuMemory");
class CCpuMemory : public CObject, IDeviceMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDeviceMemory)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PMemory* pMemory = CData<PMemory>(pData);
        if(pMemory == nullptr || pMemory->size < 1) {
            return sCtx.error("创建内存参数无效");
        }

        m_nSize = pMemory->size;
        m_spTaker.take(new unsigned char[m_nSize], [](unsigned char* pBuffer){
            delete[] pBuffer;
        });
        if(pMemory->pByteArray != nullptr) {
            memcpy( (unsigned char*)m_spTaker, pMemory->pByteArray, pMemory->size);
        }
        return sCtx.success();
    }

protected://IArchivable
    int getClassVer() { return 220308; }
    const char* getClassKey() { return CCpuMemory::__getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("size", m_nSize);
        return ar.visitTaker("data", m_nSize, m_spTaker);
    }

private://IDeviceMemory
    int getDevice(SDevice& spDevice){
        return SDeviceFactory::getFactory()->getCpuDevice(spDevice);
    }

    int getMemory(PMemory& devcieMemory){
        if(!m_spTaker) {
            return sCtx.error();
        }
        devcieMemory.size = m_nSize;
        devcieMemory.data = m_spTaker;
        return sCtx.success();
    }

    int setDeviceMemory(PMemory cpuDeviceMemory, int iOffset=0){
        if(!m_spTaker) {
            return sCtx.error();
        }
        if(cpuDeviceMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        memcpy((unsigned char*)m_spTaker + iOffset, cpuDeviceMemory.pByteArray, cpuDeviceMemory.size);
        return sCtx.success();
    }

    int getDeviceMemory(PMemory cpuDeviceMemory, int iOffset=0){
        if(!m_spTaker) {
            return sCtx.error();
        }
        if(cpuDeviceMemory.size + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        memcpy(cpuDeviceMemory.pByteArray, (unsigned char*)m_spTaker + iOffset , cpuDeviceMemory.size);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuMemory"; }

private:
    int m_nSize;
    CTaker<unsigned char*> m_spTaker;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CCpuMemory, CCpuMemory::__getClassKey())