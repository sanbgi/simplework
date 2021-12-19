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
    virtual int initVector( SData::tid eElementType, int nElementSize, void* pElementData) = 0;
    virtual int initTensor( const STensor& spDimVector, SData::tid eElementType, int nElementSize, void* pElementData) = 0;
};

//
// 张量模板类
//
template<typename T> class CTensor : public CPlaceTensor {

public://ITensor
    int initVector(SData::tid eDt, int nSize, void* pData) {

        if( eDt != getDataType() || pData == nullptr ) {
            return SError::ERRORTYPE_FAILURE;
        }

        release();

        m_spElementData.take(new T[nSize], [](T* pPtr){ delete[] pPtr;});
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
        m_nElementSize = nSize;
        return SError::ERRORTYPE_SUCCESS;
    }

    int initTensor( const STensor& spDimVector, SData::tid eElementType, int nElementSize, void* pElementData = nullptr) {

        if( eElementType != getDataType() ) {
            return SError::ERRORTYPE_FAILURE;
        }

        if( spDimVector ) {
            if( spDimVector->getDataType() != SData::getBasicTypeIdentifier<int>() ) {
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
        
        if( initVector(eElementType, nElementSize, pElementData) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        m_spDimVector = spDimVector;
        return SError::ERRORTYPE_SUCCESS;
    }

    const STensor& getDimVector() {
        if(!m_spDimVector && m_nElementSize > 0) {
            m_spDimVector = STensor::createVector(1, &m_nElementSize);
        }
        return m_spDimVector;
    }

public://ITensor

    SData::tid getDataType() const {
        return SData::getBasicTypeIdentifier<T>();
    }

    int getDataSize() const {
        return m_nElementSize;
    }

    const void* getDataPtr(SData::tid eElementType, int iPos=0) const {
        if( eElementType == getDataType() ){
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
std::map<SData::tid, FCreateTensor> g_tensorFacotries = {
    { SData::getBasicTypeIdentifier<bool>(), CTensor<bool>::createTensor },
    { SData::getBasicTypeIdentifier<char>(), CTensor<char>::createTensor },
    { SData::getBasicTypeIdentifier<unsigned char>(), CTensor<unsigned char>::createTensor },
    { SData::getBasicTypeIdentifier<short>(), CTensor<short>::createTensor },
    { SData::getBasicTypeIdentifier<int>(), CTensor<int>::createTensor },
    { SData::getBasicTypeIdentifier<long>(), CTensor<long>::createTensor },
    { SData::getBasicTypeIdentifier<float>(), CTensor<float>::createTensor },
    { SData::getBasicTypeIdentifier<double>(), CTensor<double>::createTensor },
};

//
// 定义一个张量工厂类
//
class CTensorFactory : public CObject, STensor::ITensorFactory {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(STensor::ITensorFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://ITensor
    CPlaceTensor* createTensor(SData::tid tid, SObject& rObject) {
        std::map<SData::tid, FCreateTensor>::iterator it = g_tensorFacotries.find(tid);
        if( it != g_tensorFacotries.end() ) {
            return (*(it->second))(rObject);
        }
        
        return nullptr;
    }

    STensor createVector( SData::tid eElementType, int nElementSize, void* pElementData) {
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(eElementType, spObject);
        if(pTensor) {
            if( pTensor->initVector(eElementType, nElementSize, pElementData) == SError::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }

    STensor createTensor( const STensor& spDimVector, SData::tid eElementType, int nElementSize, void* pElementData){
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(eElementType, spObject);
        if(pTensor) {
            if( pTensor->initTensor(spDimVector, eElementType, nElementSize, pElementData) == SError::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CTensorFactory, STensor::STensorFactory::getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE