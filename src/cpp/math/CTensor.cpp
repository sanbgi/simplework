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

    enum EArMode {
        None,
        Block,
        Object,
    };
    static EArMode getArMode(unsigned idType) {
        static map<unsigned int, EArMode> s_assistMap = {
            { CBasicData<bool>::getStaticType(), EArMode::Block },
            { CBasicData<char>::getStaticType(), EArMode::Block },
            { CBasicData<unsigned char>::getStaticType(), EArMode::Block },
            { CBasicData<short>::getStaticType(), EArMode::Block },
            { CBasicData<int>::getStaticType(), EArMode::Block },
            { CBasicData<long>::getStaticType(), EArMode::Block },
            { CBasicData<unsigned int>::getStaticType(), EArMode::Block },
            { CBasicData<float>::getStaticType(), EArMode::Block },
            { CBasicData<double>::getStaticType(), EArMode::Block },
            { CData<SObject>::getStaticType(), EArMode::Object },
        };
        map<unsigned int, EArMode>::iterator it = s_assistMap.find(idType);
        if( it == s_assistMap.end() ) {
            return EArMode::None;
        }
        return it->second;
    }
    static CTypeAssist* getTypeAssist(unsigned int idType) {
        static map<unsigned int, CTypeAssist*> s_assistMap = {
            { CBasicData<bool>::getStaticType(), getTypeAssist<bool>() },
            { CBasicData<char>::getStaticType(), getTypeAssist<char>() },
            { CBasicData<unsigned char>::getStaticType(), getTypeAssist<unsigned char>() },
            { CBasicData<short>::getStaticType(), getTypeAssist<short>() },
            { CBasicData<int>::getStaticType(), getTypeAssist<int>() },
            { CBasicData<long>::getStaticType(), getTypeAssist<long>() },
            { CBasicData<unsigned int>::getStaticType(), getTypeAssist<unsigned int>() },
            { CBasicData<float>::getStaticType(), getTypeAssist<float>() },
            { CBasicData<double>::getStaticType(), getTypeAssist<double>() },
            { CData<SObject>::getStaticType(), getTypeAssist<SObject>() },
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
    template<typename Q> static CTypeAssist* getTypeAssist() {
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
                switch(CTypeAssist::getArMode(m_idType)) {
                case EArMode::Block:
                    {
                        if(ar->isReading()) {
                            spTaker.take(new Q[nSize], CTypeAssist::FreeMemory<Q>);
                        }
                        ar.arBlockArray("data", nSize, (Q*)(void*)spTaker);
                    }
                    break;

                case EArMode::Object: 
                    {
                        if(ar->isReading()) {
                            spTaker.take(new SObject[nSize], CTypeAssist::FreeMemory<SObject>);
                        }
                        SObject* pQ = (SObject*)(void*)spTaker;
                        while(nSize-->0) {
                            ar.arObject("data", *pQ);
                            pQ++;
                        }
                    }
                    break;
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
    if( spTensor->initVector(CTypeAssist::getTypeAssist<int>(), nDims, (void*)pDimSizes) != sCtx.success()) {
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