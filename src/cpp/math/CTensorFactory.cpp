#include "CTensorFactory.h"

using namespace sw;
using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensor");

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CPlaceTensor : public CObject, ITensor {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    virtual int initVector( unsigned int idElementType, int nElementSize, void* pElementData) = 0;
    virtual int initTensor( const STensor& spDimVector, unsigned int idElementType, int nElementSize, void* pElementData) = 0;
};

//
// 张量模板类
//
template<typename T> class CTensor : public CPlaceTensor {

public://ITensor
    int initVector(unsigned int eDt, int nSize, void* pData) {

        if( eDt != getDataType() ) {
            return SError::ERRORTYPE_FAILURE;
        }

        release();
        m_spElementData.take(new T[nSize], [](T* pPtr){ 
            delete[] pPtr;
        });
        if(pData) {
            if(true) {//SData::isPuryMemoryType(eDt)) {
                memcpy((T*)m_spElementData, (T*)pData, nSize*sizeof(T));
            }
            else {
                T* pSrc = (T*)pData;
                T* pDesc = (T*)m_spElementData;
                while(nSize--) {
                    *pSrc = *pDesc;
                    pSrc++, pDesc++;
                }
            }
        }
        m_nElementSize = nSize;
        return SError::ERRORTYPE_SUCCESS;
    }

    int initTensor( const STensor& spDimVector, unsigned int idElementType, int nElementSize, void* pElementData = nullptr) {

        if( idElementType != getDataType() ) {
            return SError::ERRORTYPE_FAILURE;
        }

        if( spDimVector ) {
            if( spDimVector->getDataType() != CBasicData<int>::getStaticType() ) {
                return SError::ERRORTYPE_FAILURE;
            }

            int nDim = spDimVector->getDataSize();
            const int* pDimSize = spDimVector->getDataPtr<int>();
            int nSize = 1;
            for( int i=0; i<nDim; i++) {
                if(pDimSize[i] < 1) {
                    return SError::ERRORTYPE_FAILURE;
                }
                nSize *= pDimSize[i];
            }
            if(nSize!= nElementSize) {
                return SError::ERRORTYPE_FAILURE;
            }
        }
        
        if( initVector(idElementType, nElementSize, pElementData) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        m_spDimVector = spDimVector;
        return SError::ERRORTYPE_SUCCESS;
    }

    STensor& getDimVector() {
        if(!m_spDimVector && m_nElementSize > 0) {
            STensor::createVector(m_spDimVector, 1, &m_nElementSize);
        }
        return m_spDimVector;
    }

public://ITensor

    int getVer() {
        return m_nVer;
    }

    int updateVer() {
        return ++m_nVer;
    }

    unsigned int getDataType() {
        return CBasicData<T>::getStaticType();
    }

    int getDataSize() {
        return m_nElementSize;
    }

    void* getDataPtr(unsigned int idElementType, int iPos=0) {
        if( idElementType == getDataType() ){
            if( iPos >= 0 && iPos < m_nElementSize ) {
                return (T*)m_spElementData + iPos;
            }
        }
        return nullptr;
    }

public:
    CTensor() {
        m_nVer = 0;
        m_nElementSize = 0;
    }
    ~CTensor() {
        release();
    }
    void release() {
        m_spElementData.release();
        m_nElementSize = 0;
    }

public:
    static CPlaceTensor* createTensor(SObject& rObject) {
        CPointer<CTensor<T>> spTensor;
        CObject::createObject(spTensor);
        rObject = spTensor.getObject();
        return spTensor;
    }

private:
    int m_nVer;
    int m_nElementSize;
    CTaker<T*> m_spElementData;
    STensor m_spDimVector;
};


typedef CPlaceTensor* (*FCreateTensor)(SObject& spTensor);
std::map<unsigned int, FCreateTensor> g_tensorFacotries = {
    { CBasicData<bool>::getStaticType(), CTensor<bool>::createTensor },
    { CBasicData<char>::getStaticType(), CTensor<char>::createTensor },
    { CBasicData<unsigned char>::getStaticType(), CTensor<unsigned char>::createTensor },
    { CBasicData<short>::getStaticType(), CTensor<short>::createTensor },
    { CBasicData<int>::getStaticType(), CTensor<int>::createTensor },
    { CBasicData<long>::getStaticType(), CTensor<long>::createTensor },
    { CBasicData<float>::getStaticType(), CTensor<float>::createTensor },
    { CBasicData<double>::getStaticType(), CTensor<double>::createTensor },
};

CPlaceTensor* createTensor(unsigned int tid, SObject& rObject) {
    std::map<unsigned int, FCreateTensor>::iterator it = g_tensorFacotries.find(tid);
    if( it != g_tensorFacotries.end() ) {
        return (*(it->second))(rObject);
    }
    
    return nullptr;
}

int CTensorFactory::createVector( STensor& spTensor, unsigned int idElementType, int nElementSize, void* pElementData) {
    SObject spObject;
    CPlaceTensor* pTensor = ::createTensor(idElementType, spObject);
    if(pTensor) {
        if( int errCode = pTensor->initVector(idElementType, nElementSize, pElementData) != SError::ERRORTYPE_SUCCESS) {
            return errCode;
        }
        spTensor.setPtr((ITensor*)pTensor);
        return sCtx.success();
    }
    return sCtx.error("无法创建指定类型的张量");
}

int CTensorFactory::createTensor( STensor& spTensor, const STensor& spDimVector, unsigned int idElementType, int nElementSize, void* pElementData){
    SObject spObject;
    CPlaceTensor* pTensor = ::createTensor(idElementType, spObject);
    if(pTensor) {
        if( int errCode = pTensor->initTensor(spDimVector, idElementType, nElementSize, pElementData) != SError::ERRORTYPE_SUCCESS) {
            return errCode;
        }
        spTensor.setPtr((ITensor*)pTensor);
        return sCtx.success();
    }
    return sCtx.error("无法创建指定类型的张量");
}

SIMPLEWORK_MATH_NAMESPACE_LEAVE