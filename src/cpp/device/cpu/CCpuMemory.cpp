#include "CpuDevice.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CCpuMemory");
class CCpuMemory : public CObject, public IKernelMemory, IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IKernelMemory)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://CObject
    int __initialize(const PData* pData){
        const PMemory* pMemory = CData<PMemory>(pData);
        if(pMemory == nullptr || pMemory->size < 1) {
            return sCtx.error("创建内存参数无效");
        }
        return initialize(pMemory);
    }

public:
    int initialize(const PMemory* pMemory){
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

private://IKernelMemory
    int getSize() {
        return m_nSize;
    }

    int getDevice(SDevice& spDevice){
        spDevice = SDevice::cpu();
        return sCtx.success();
    }

    void* getPointer(const SDevice& spDevice){
        if(spDevice.getPtr() == SDevice::cpu().getPtr()){
            return m_spTaker;
        }
        return nullptr;
    }

    int writeMemory(const SKernelMemory& spMemory) {
        if(spMemory.getPtr() == this) {
            return sCtx.success();
        }

        if(spMemory.size() != m_nSize) {
            return sCtx.error("不能写入大小不一样的内存");
        }

        return spMemory->readMemory(m_nSize, m_spTaker);
    }

    int writeMemory(int nSize, void* pData, int iOffset=0){
        if(!m_spTaker) {
            return sCtx.error();
        }
        if(nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        memcpy((unsigned char*)m_spTaker + iOffset, pData, nSize);
        return sCtx.success();
    }

    int readMemory(int nSize, void* pData, int iOffset=0){
        if(!m_spTaker) {
            return sCtx.error();
        }
        if(nSize + iOffset > m_nSize) {
            return sCtx.error("设置内存超出了范围");
        }
        memcpy(pData, (unsigned char*)m_spTaker + iOffset , nSize);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.device.CpuMemory"; }

private:
    int m_nSize;
    CTaker<unsigned char*> m_spTaker;
};

int createMemory(PMemory initMemory, SKernelMemory& spMempry) {
    CPointer<CCpuMemory> spPointer;
    CObject::createObject(spPointer);
    int ret = spPointer->initialize(&initMemory);
    if( ret != sCtx.success() ) {
        return ret;
    }
    spMempry.setPtr(spPointer.getPtr());
    return ret;
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CCpuMemory, CCpuMemory::__getClassKey())