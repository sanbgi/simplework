#include "CTensor.h"
#include "CDimension.h"

#include <map>
using namespace std;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensor");

class CTypeAssist {
public:
    unsigned int m_idType;
    int m_nTypeBytes;
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
        };
        map<unsigned int, CTypeAssist*>::iterator it = s_assistMap.find(idType);
        if( it == s_assistMap.end() ) {
            return nullptr;
        }
        return it->second;
    }
    template<typename Q> static CTypeAssist* getTypeAssist() {
        static class CTypeAssistT : public CTypeAssist {
        public:
            CTypeAssistT() {
                m_idType = CBasicData<Q>::getStaticType();
                m_nTypeBytes = sizeof(Q);
            }
        }s_assist;
        return &s_assist;
    }

    static int archiveAssist(CTypeAssist** ppAssist, const SIoArchive& ar) {
        if(ar->isReading()) {
            unsigned int idType;
            ar.visit("idType", idType);
            *ppAssist = getTypeAssist(idType);
        }else{
            ar.visit("idType", (*ppAssist)->m_idType);
        }

        return sCtx.success();
    }
};

int CTensor::initVector(CTypeAssist* pTypeAssist, int nSize, void* pData) {

    release();
    int nBytes = nSize * pTypeAssist->m_nTypeBytes;
    m_spElementData.take(new char[nBytes], [](char* pPtr){ 
        delete[] pPtr;
    });
    if(pData) {
        memcpy(m_spElementData, pData, nBytes);
    }
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
        if( iPos >= 0 && iPos < m_nElementSize ) {
            return (char*)m_spElementData + iPos * m_pTypeAssist->m_nTypeBytes;
        }
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

int CTensor::toArchive(const SIoArchive& ar) {
    CTypeAssist::archiveAssist(&m_pTypeAssist, ar);
    ar.visit("ver", m_nVer);
    ar.visit("size", m_nElementSize);
    ar.visitTaker("data", m_nElementSize * m_pTypeAssist->m_nTypeBytes, m_spElementData);
    ar.visitObject("dimension", m_spDimVector);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CTensor, CTensor::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE