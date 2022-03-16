#include "CTensor.h"
#include "CDimension.h"

#include <map>
#include <vector>
using namespace std;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensor");

class CTypeAssist {
public:
    PDATATYPE m_idType;
    static CTypeAssist* getTypeAssist(PDATATYPE idType) {
        static map<PDATATYPE, CTypeAssist*> s_assistMap = {
            { CBasicData<bool>::getStaticType(), getBasicTypeAssist<bool>() },
            { CBasicData<char>::getStaticType(), getBasicTypeAssist<char>() },
            { CBasicData<unsigned char>::getStaticType(), getBasicTypeAssist<unsigned char>() },
            { CBasicData<short>::getStaticType(), getBasicTypeAssist<short>() },
            { CBasicData<int>::getStaticType(), getBasicTypeAssist<int>() },
            { CBasicData<long>::getStaticType(), getBasicTypeAssist<long>() },
            { CBasicData<unsigned int>::getStaticType(), getBasicTypeAssist<unsigned int>() },
            { CBasicData<float>::getStaticType(), getBasicTypeAssist<float>() },
            { CBasicData<double>::getStaticType(), getBasicTypeAssist<double>() },
            //{ CData<SObject>::getStaticType(), getObjectTypeAssist<SObject>() },
        };
        map<PDATATYPE, CTypeAssist*>::iterator it = s_assistMap.find(idType);
        if( it == s_assistMap.end() ) {
            return nullptr;
        }
        return it->second;
    }

    template<typename Q> static void FreeMemory(void* pQ) {
        delete[] (Q*)pQ;
    }

    template<typename Q> static CTypeAssist* getBasicTypeAssist() {
        static class CTypeAssistT : public CTypeAssist {
        public:
            CTypeAssistT() {
                m_idType = CBasicData<Q>::getStaticType();
            }

            void initData(CTaker<void*>& spTaker, int nSize, const void* pData) {
                spTaker.take(new Q[nSize], CTypeAssist::FreeMemory<Q>);
                if(pData != nullptr) {
                    Q* pDest = (Q*)(void*)spTaker;
                    Q* pSrc = (Q*)pData;
                    while(nSize-->0) {
                        *pDest = *pSrc;
                        pDest++, pSrc++;
                    }
                }
            }


            void archiveData(const SArchive& ar, CTaker<void*>& spTaker, int nSize) {
                if(ar->isReading()) {
                    spTaker.take(new Q[nSize], CTypeAssist::FreeMemory<Q>);
                }
                ar.arBlockArray("data", nSize, (Q*)(void*)spTaker);
            }

            int size() { 
                return sizeof(Q);
            }
        }s_assist;
        return &s_assist;
    }

    template<typename Q> static CTypeAssist* getObjectTypeAssist() {
        static class CTypeAssistT : public CTypeAssist {
        public:
            CTypeAssistT() {
                m_idType = CData<Q>::getStaticType();
            }

            void initData(CTaker<void*>& spTaker, int nSize, const void* pData) {
                spTaker.take(new Q[nSize], CTypeAssist::FreeMemory<Q>);
                if(pData != nullptr) {
                    Q* pDest = (Q*)(void*)spTaker;
                    Q* pSrc = (Q*)pData;
                    while(nSize-->0) {
                        *pDest = *pSrc;
                        pDest++, pSrc++;
                    }
                }
            }

            void archiveData(const SArchive& ar, CTaker<void*>& spTaker, int nSize) {
                if(ar->isReading()) {
                    spTaker.take(new Q[nSize], CTypeAssist::FreeMemory<Q>);
                }
                Q* pQ = (Q*)(void*)spTaker;
                while(nSize-->0) {
                    ar.arObject("data", *pQ);
                    pQ++;
                }
            }

            int size() { 
                return sizeof(Q);
            }
        }s_assist;
        return &s_assist;
    }

    static int archiveAssist(CTypeAssist** ppAssist, const SArchive& ar) {
        if(ar->isReading()) {
            PDATATYPE idType;
            ar.arBlock("idType", idType);
            *ppAssist = getTypeAssist(idType);
        }else{
            ar.arBlock("idType", (*ppAssist)->m_idType);
        }
        return sCtx.success();
    }

    virtual void initData(CTaker<void*>& spTaker, int nSize, const void* pData) = 0;
    virtual void archiveData(const SArchive& ar, CTaker<void*>& spTaker, int nSize) = 0;
    virtual int size() = 0;
};

int CTensor::initVector(CTypeAssist* pTypeAssist, int nSize, const void* pData) {
    m_spMemory = SDeviceMemory::createDeviceMemory(SDevice::defaultHostDevice(), nSize*pTypeAssist->size(), (void*)pData);
    m_pTypeAssist = pTypeAssist;
    m_nElementSize = nSize;
    return SError::ERRORTYPE_SUCCESS;
}

int CTensor::initTensor(CTypeAssist* pTypeAssist, const SDimension& spDimVector, int nElementSize, const void* pElementData) {
    if( spDimVector ) {
        if(nElementSize!= spDimVector->getElementSize()) {
            return SError::ERRORTYPE_FAILURE;
        }
    }
    
    if( initVector(pTypeAssist, nElementSize, pElementData) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_spDimVector = spDimVector;
    return SError::ERRORTYPE_SUCCESS;
}

SDimension& CTensor::getDimVector() {
    if(!m_spDimVector && m_nElementSize > 0) {
        SDimension::createDimension(m_spDimVector, 1, &m_nElementSize);
    }
    return m_spDimVector;
}

int CTensor::getDimension(SDimension& spDim) {
    spDim = getDimVector();
    return sCtx.success();
}


int CTensor::getDataBuffer(SDeviceMemory& spMemory) {
    spMemory = m_spMemory;
    return sCtx.success();
}

PDATATYPE CTensor::getDataType() {
    return m_pTypeAssist->m_idType;
}

int CTensor::getDataSize() {
    return m_nElementSize;
}

CTensor::CTensor() {
    m_nElementSize = 0;
}

int CTensor::createTensor(STensor& spTensor, const SDimension* pDimension, PDATATYPE idElementType, int nElementSize, const void* pElementData) {
    CTypeAssist* pTypeAssist = CTypeAssist::getTypeAssist(idElementType);
    if(pTypeAssist == nullptr) {
        return sCtx.error("不支持指定类型的张量");
    }

    CPointer<CTensor> sp;
    CObject::createObject(sp);
    if(pDimension) {
        if( sp->initTensor(pTypeAssist, *pDimension, nElementSize, pElementData) != sCtx.success() ) {
            return sCtx.error("初始化张量失败");
        }
    }else{
        if( sp->initVector(pTypeAssist, nElementSize, pElementData) != sCtx.success() ) {
            return sCtx.error("初始化张量失败");
        }
    }
    spTensor.setPtr(sp.getPtr());
    return sCtx.success();
}

int CTensor::createTensor(STensor& spTensor, PDATATYPE eElementType, int nElementSize, const SDimension& spDimension, const SDeviceMemory& spDataBuffer) {
    CTypeAssist* pTypeAssist = CTypeAssist::getTypeAssist(eElementType);
    if(pTypeAssist == nullptr) {
        return sCtx.error("不支持指定类型的张量");
    }

    if( spDimension && spDimension.dataSize() != nElementSize ) {
        return sCtx.error("维度定义与数据数量不一致");
    }

    if( spDataBuffer && spDataBuffer.size() != pTypeAssist->size() * nElementSize ) {
        return sCtx.error("数据缓冲大小与数据量不一致");
    }

    CPointer<CTensor> sp;
    CObject::createObject(sp);
    sp->m_pTypeAssist = pTypeAssist;
    sp->m_nElementSize = nElementSize;
    sp->m_spDimVector = spDimension;
    sp->m_spMemory = spDataBuffer;
    spTensor.setPtr(sp.getPtr());
    return sCtx.success();
}


int CTensor::toArchive(const SArchive& ar) {
    CTypeAssist::archiveAssist(&m_pTypeAssist, ar);
    ar.arBlock("size", m_nElementSize);
    ar.arObject("data", m_spMemory);
    ar.arObject("dimension", m_spDimVector);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CTensor, CTensor::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE