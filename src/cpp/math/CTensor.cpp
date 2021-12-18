#include "../inc/math/math.h"

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
    virtual int initVector( SData::DataType eElementType, int nElementSize, void* pElementData) = 0;
    virtual int initTensor( const STensor& spDimVector, SData::DataType eElementType, int nElementSize, void* pElementData) = 0;
};

//
// 张量模板类
//
template<typename T> class CTensor : public CPlaceTensor {

public://ITensor
    int initVector(SData::DataType eDt, int nSize, void* pData) {

        if( eDt != getDataType() || pData == nullptr ) {
            return SError::ERRORTYPE_FAILURE;
        }

        release();

        m_spElementData.take(new T[nSize], [](T* pPtr){ delete[] pPtr;});
        if(SData::isPuryMemoryType(eDt)) {
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

    int initTensor( const STensor& spDimVector, SData::DataType eElementType, int nElementSize, void* pElementData = nullptr) {

        if( eElementType != getDataType() ) {
            return SError::ERRORTYPE_FAILURE;
        }

        if( spDimVector ) {
            if( spDimVector->getDataType() != SData::DATATYPE_INT ) {
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

    SData::DataType getDataType() const {
        return SData::getType<T>();
    }

    int getDataSize() const {
        return m_nElementSize;
    }

    const void* getDataPtr(SData::DataType eElementType, int iPos=0) const {
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

private:
    int m_nElementSize;
    CTaker<T*> m_spElementData;
    STensor m_spDimVector;
};

//
// 定义一个张量工厂类
//
class CTensorFactory : public CObject, STensor::ITensorFactory {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(STensor::ITensorFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://ITensor
    template<typename TType> CPlaceTensor* createTensor(SObject& rObject) {
        CPointer<CTensor<TType>> spTensor;
        CObject::createObject(spTensor);
        rObject = spTensor.getObject();
        return spTensor;
    }
    
    CPlaceTensor* createTensor(SData::DataType eElementType, SObject& rObject) {
        switch(eElementType) {
            case SData::DATATYPE_BOOL:
                return createTensor<bool>(rObject);
            case SData::DATATYPE_CHAR:
                return createTensor<char>(rObject);
            case SData::DATATYPE_UCHAR:
                return createTensor<unsigned char>(rObject);
            case SData::DATATYPE_SHORT:
                return createTensor<short>(rObject);
            case SData::DATATYPE_INT:
                return createTensor<int>(rObject);
            case SData::DATATYPE_LONG:
                return createTensor<long>(rObject);
            case SData::DATATYPE_FLOAT:
                return createTensor<float>(rObject);
            case SData::DATATYPE_DOUBLE:
                return createTensor<double>(rObject);
            case SData::DATATYPE_OBJECT:
                return createTensor<SObject>(rObject);
        }
        return nullptr;
    }

    STensor createVector( SData::DataType eElementType, int nElementSize, void* pElementData) {
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(eElementType, spObject);
        if(pTensor) {
            if( pTensor->initVector(eElementType, nElementSize, pElementData) == SError::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }

    STensor createTensor( const STensor& spDimVector, SData::DataType eElementType, int nElementSize, void* pElementData){
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