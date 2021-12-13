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
    virtual int initVector( Data::DataType eElementType, int nElementSize, void* pElementData) = 0;
    virtual int initTensor( const STensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData) = 0;
};

//
// 张量模板类
//
template<typename T> class CTensor : public CPlaceTensor {

public://ITensor
    int initVector(Data::DataType eDt, int nSize, void* pData) {

        if( eDt != getDataType() || pData == nullptr ) {
            return Error::ERRORTYPE_FAILURE;
        }

        release();

        m_spElementData.take(new T[nSize], [](T* pPtr){ delete[] pPtr;});
        if(Data::isPuryMemoryType(eDt)) {
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
        return Error::ERRORTYPE_SUCCESS;
    }

    int initTensor( const STensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData = nullptr) {

        if( eElementType != getDataType() ) {
            return Error::ERRORTYPE_FAILURE;
        }

        if( spDimVector ) {
            if( spDimVector->getDataType() != Data::DATATYPE_INT ) {
                return Error::ERRORTYPE_FAILURE;
            }

            int nDim = spDimVector->getDataSize();
            const int* pDimSize = spDimVector->getDataPtr<int>();
            int nSize = 1;
            for( int i=0; i<nDim; i++) {
                if(pDimSize[i] < 1) {
                    return Error::ERRORTYPE_FAILURE;
                }
                nSize *= pDimSize[i];
            }
            if(nSize!= nElementSize) {
                return Error::ERRORTYPE_FAILURE;
            }
        }
        
        if( initVector(eElementType, nElementSize, pElementData) != Error::ERRORTYPE_SUCCESS ) {
            return Error::ERRORTYPE_FAILURE;
        }
        m_spDimVector = spDimVector;
        return Error::ERRORTYPE_SUCCESS;
    }

    const STensor& getDimVector() {
        if(!m_spDimVector && m_nElementSize > 0) {
            m_spDimVector = STensor::createVector(1, &m_nElementSize);
        }
        return m_spDimVector;
    }

public://ITensor

    Data::DataType getDataType() const {
        return Data::getType<T>();
    }

    int getDataSize() const {
        return m_nElementSize;
    }

    const void* getDataPtr(Data::DataType eElementType, int iPos=0) const {
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
        CPlaceTensor* pTensor = CObject::createObject<CTensor<TType>>(rObject);
        return pTensor;
    }
    
    CPlaceTensor* createTensor(Data::DataType eElementType, SObject& rObject) {
        switch(eElementType) {
            case Data::DATATYPE_BOOL:
                return createTensor<bool>(rObject);
            case Data::DATATYPE_CHAR:
                return createTensor<char>(rObject);
            case Data::DATATYPE_UCHAR:
                return createTensor<unsigned char>(rObject);
            case Data::DATATYPE_SHORT:
                return createTensor<short>(rObject);
            case Data::DATATYPE_INT:
                return createTensor<int>(rObject);
            case Data::DATATYPE_LONG:
                return createTensor<long>(rObject);
            case Data::DATATYPE_FLOAT:
                return createTensor<float>(rObject);
            case Data::DATATYPE_DOUBLE:
                return createTensor<double>(rObject);
            case Data::DATATYPE_OBJECT:
                return createTensor<SObject>(rObject);
        }
        return nullptr;
    }

    STensor createVector( Data::DataType eElementType, int nElementSize, void* pElementData) {
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(eElementType, spObject);
        if(pTensor) {
            if( pTensor->initVector(eElementType, nElementSize, pElementData) == Error::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }

    STensor createTensor( const STensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData){
        SObject spObject;
        CPlaceTensor* pTensor = createTensor(eElementType, spObject);
        if(pTensor) {
            if( pTensor->initTensor(spDimVector, eElementType, nElementSize, pElementData) == Error::ERRORTYPE_SUCCESS)
                return STensor::wrapPtr((ITensor*)pTensor);
            return STensor();
        }
        return STensor();
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CTensorFactory, STensor::STensorFactory::getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE