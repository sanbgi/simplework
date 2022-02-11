#include "CTensor.h"
#include "CDimension.h"

#include <map>
#include <vector>
using namespace std;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensor");

class CTypeAssist {
public:
    unsigned int m_idType;
    static CTypeAssist* getTypeAssist(unsigned int idType) {
        static map<unsigned int, CTypeAssist*> s_assistMap = {
            { CBasicData<bool>::getStaticType(), getBasicTypeAssist<bool>() },
            { CBasicData<char>::getStaticType(), getBasicTypeAssist<char>() },
            { CBasicData<unsigned char>::getStaticType(), getBasicTypeAssist<unsigned char>() },
            { CBasicData<short>::getStaticType(), getBasicTypeAssist<short>() },
            { CBasicData<int>::getStaticType(), getBasicTypeAssist<int>() },
            { CBasicData<long>::getStaticType(), getBasicTypeAssist<long>() },
            { CBasicData<unsigned int>::getStaticType(), getBasicTypeAssist<unsigned int>() },
            { CBasicData<float>::getStaticType(), getBasicTypeAssist<float>() },
            { CBasicData<double>::getStaticType(), getBasicTypeAssist<double>() },
            { CData<SObject>::getStaticType(), getObjectTypeAssist<SObject>() },
        };
        map<unsigned int, CTypeAssist*>::iterator it = s_assistMap.find(idType);
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

            void initData(CTaker<void*>& spTaker, int nSize, void* pData) {
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

            void* getDataPtr(CTaker<void*>& spTaker, int iPos) {
                Q* pQ = (Q*)(void*)spTaker;
                return pQ+iPos;
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

            void initData(CTaker<void*>& spTaker, int nSize, void* pData) {
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

            void* getDataPtr(CTaker<void*>& spTaker, int iPos) {
                Q* pQ = (Q*)(void*)spTaker;
                return pQ+iPos;
            }
        }s_assist;
        return &s_assist;
    }

    static int archiveAssist(CTypeAssist** ppAssist, const SArchive& ar) {
        if(ar->isReading()) {
            unsigned int idType;
            ar.arBlock("idType", idType);
            *ppAssist = getTypeAssist(idType);
        }else{
            ar.arBlock("idType", (*ppAssist)->m_idType);
        }
        return sCtx.success();
    }

    virtual void initData(CTaker<void*>& spTaker, int nSize, void* pData) = 0;
    virtual void* getDataPtr(CTaker<void*>& spTaker, int iPos) = 0;
    virtual void archiveData(const SArchive& ar, CTaker<void*>& spTaker, int nSize) = 0;
};

int CTensor::initVector(CTypeAssist* pTypeAssist, int nSize, void* pData) {
    release();
    pTypeAssist->initData(m_spElementData, nSize, pData);
    m_pTypeAssist = pTypeAssist;
    m_nElementSize = nSize;
    return SError::ERRORTYPE_SUCCESS;
}

int CTensor::initTensor(CTypeAssist* pTypeAssist, const SDimension& spDimVector, int nElementSize, void* pElementData) {
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

int CTensor::getVer() {
    return m_nVer;
}

int CTensor::updateVer() {
    return ++m_nVer;
}

int CTensor::getDimension(SDimension& spDim) {
    spDim = getDimVector();
    return sCtx.success();
}

unsigned int CTensor::getDataType() {
    return m_pTypeAssist->m_idType;
}

int CTensor::getDataSize() {
    return m_nElementSize;
}

void* CTensor::getDataPtr(unsigned int idElementType, int iPos) {
    if( idElementType == getDataType() ){
        return m_pTypeAssist->getDataPtr(m_spElementData, iPos);
    }
    return nullptr;
}

CTensor::CTensor() {
    m_nVer = 0;
    m_nElementSize = 0;
}
CTensor::~CTensor() {
    release();
}
void CTensor::release() {
    m_spElementData.release();
    m_nElementSize = 0;
}

int CTensor::createTensor(STensor& spTensor, const SDimension* pDimension, unsigned int idElementType, int nElementSize, void* pElementData) {
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

int CTensor::createDimension(SDimension& spDim, int nDims, const int* pDimSizes) {
    CPointer<CDimension> spTensor;
    CObject::createObject(spTensor);
    if( spTensor->initVector(CTypeAssist::getBasicTypeAssist<int>(), nDims, (void*)pDimSizes) != sCtx.success()) {
        return sCtx.error("创建维度失败");
    }
    spDim.setPtr(spTensor.getPtr());
    return sCtx.success();
}

int CTensor::toArchive(const SArchive& ar) {
    CTypeAssist::archiveAssist(&m_pTypeAssist, ar);
    ar.arBlock("ver", m_nVer);
    ar.arBlock("size", m_nElementSize);
    m_pTypeAssist->archiveData(ar, m_spElementData, m_nElementSize);
    ar.arObject("dimension", m_spDimVector);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CTensor, CTensor::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE