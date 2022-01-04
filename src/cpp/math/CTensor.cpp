#include "../inc/math/math.h"
#include <map>

using namespace sw::core;
using namespace sw::math;

SIMPLEWORK_MATH_NAMESPACE_ENTER

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

        if( eDt != getDataType() || pData == nullptr ) {
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
            if( spDimVector->getDataType() != CBasicType<int>::getThisType() ) {
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
            m_spDimVector = STensor::createVector(1, &m_nElementSize);
        }
        return m_spDimVector;
    }

public://ITensor

    unsigned int getDataType() {
        return CBasicType<T>::getThisType();
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
    int m_nElementSize;
    CTaker<T*> m_spElementData;
    STensor m_spDimVector;
};


typedef CPlaceTensor* (*FCreateTensor)(SObject& spTensor);
std::map<unsigned int, FCreateTensor> g_tensorFacotries = {
    { CBasicType<bool>::getThisType(), CTensor<bool>::createTensor },
    { CBasicType<char>::getThisType(), CTensor<char>::createTensor },
    { CBasicType<unsigned char>::getThisType(), CTensor<unsigned char>::createTensor },
    { CBasicType<short>::getThisType(), CTensor<short>::createTensor },
    { CBasicType<int>::getThisType(), CTensor<int>::createTensor },
    { CBasicType<long>::getThisType(), CTensor<long>::createTensor },
    { CBasicType<float>::getThisType(), CTensor<float>::createTensor },
    { CBasicType<double>::getThisType(), CTensor<double>::createTensor },
};

//
// 定义一个张量工厂类
//
class CTensorFactory : public CObject, STensor::ITensorFactory {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(STensor::ITensorFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://ITensor
    CPlaceTensor* createTensor(unsigned int tid, SObject& rObject) {
        std::map<unsigned int, FCreateTensor>::iterator it = g_tensorFacotries.find(tid);
        if( it != g_tensorFacotries.end() ) {
            return (*(it->second))(rObject);
        }
        
        return nullptr;
    }

    STensor createVector( unsigned int idElementType, int nElementSize, void* pElementData) {
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(idElementType, spObject);
        if(pTensor) {
            if( pTensor->initVector(idElementType, nElementSize, pElementData) == SError::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }

    STensor createTensor( const STensor& spDimVector, unsigned int idElementType, int nElementSize, void* pElementData){
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(idElementType, spObject);
        if(pTensor) {
            if( pTensor->initTensor(spDimVector, idElementType, nElementSize, pElementData) == SError::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorFactory, STensor::STensorFactory::getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE