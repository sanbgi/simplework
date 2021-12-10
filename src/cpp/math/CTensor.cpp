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
    virtual int initTensor( const Tensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData) = 0;
};

//
// 张量模板类
//
template<typename T> class CTensor : public CPlaceTensor {

public://ITensor
    ObjectWithPtr<CPlaceTensor> createTensor(Data::DataType eElementType) {
        switch(eElementType) {
            case Data::DATATYPE_BOOL:
                return CObject::createObjectWithPtr<CTensor<bool>, CPlaceTensor>();
            case Data::DATATYPE_CHAR:
                return CObject::createObjectWithPtr<CTensor<char>, CPlaceTensor>();
            case Data::DATATYPE_UCHAR:
                return CObject::createObjectWithPtr<CTensor<unsigned char>, CPlaceTensor>();
            case Data::DATATYPE_SHORT:
                return CObject::createObjectWithPtr<CTensor<short>, CPlaceTensor>();
            case Data::DATATYPE_INT:
                return CObject::createObjectWithPtr<CTensor<int>, CPlaceTensor>();
            case Data::DATATYPE_LONG:
                return CObject::createObjectWithPtr<CTensor<long>, CPlaceTensor>();
            case Data::DATATYPE_FLOAT:
                return CObject::createObjectWithPtr<CTensor<float>, CPlaceTensor>();
            case Data::DATATYPE_DOUBLE:
                return CObject::createObjectWithPtr<CTensor<double>, CPlaceTensor>();
            case Data::DATATYPE_OBJECT:
                return CObject::createObjectWithPtr<CTensor<Object>, CPlaceTensor>();
        }
        return ObjectWithPtr<CPlaceTensor>();
    }

    Tensor createVector( Data::DataType eElementType, int nElementSize, void* pElementData) {
        ObjectWithPtr<CPlaceTensor> spWrapTensor = createTensor(eElementType);
        if(spWrapTensor.spObject) {
            if( spWrapTensor.pObject->initVector(eElementType, nElementSize, pElementData) )
                return Tensor::wrapPtr((ITensor*)spWrapTensor.pObject);
            return Tensor();
        }
        return spWrapTensor.spObject;
    }

    Tensor createTensor( const Tensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData){
        ObjectWithPtr<CPlaceTensor> spWrapTensor = createTensor(eElementType);
        if(spWrapTensor.spObject) {
            if( spWrapTensor.pObject->initTensor(spDimVector, eElementType, nElementSize, pElementData) )
                return Tensor::wrapPtr((ITensor*)spWrapTensor.pObject);
            return Tensor();
        }
        return spWrapTensor.spObject;
    }

    int initVector(Data::DataType eDt, int nSize, void* pData) {

        if( eDt != getDataType() || pData == nullptr ) {
            return Error::ERRORTYPE_FAILURE;
        }

        release();
        m_pElementData = new T[nSize];
        if(Data::isPuryMemoryType(eDt)) {
            memcpy(m_pElementData, (T*)pData, nSize*sizeof(T));
        }
        else {
            T* pSrc = (T*)pData;
            T* pDesc = m_pElementData;
            while(nSize--) {
                *pSrc = *pDesc;
                pSrc++, pDesc++;
            }
        }
        m_nElementSize = nSize;
        return Error::ERRORTYPE_SUCCESS;
    }

    int initTensor( const Tensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData = nullptr) {

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

    const Tensor& getDimVector() {
        if(!m_spDimVector && m_nElementSize > 0) {
            m_spDimVector = Tensor::createVector(1, &m_nElementSize);
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
            if( iPos >= 0 && iPos < m_nElementSize )
                return m_pElementData + iPos;
        }
        return nullptr;
    }

public:
    CTensor() {
        m_pElementData = nullptr;
        m_nElementSize = 0;
    }
    ~CTensor() {
        release();
    }
    void release() {
        if(m_pElementData) {
            delete[] m_pElementData;
            m_pElementData = nullptr;
        }
        m_nElementSize = 0;
    }

private:
    int m_nElementSize;
    T* m_pElementData;
    Tensor m_spDimVector;
};

//
// 定义一个张量工厂类
//
typedef CTensor<bool> CFactoryTensor;

SIMPLEWORK_FACTORY_REGISTER(CFactoryTensor, Tensor::getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE