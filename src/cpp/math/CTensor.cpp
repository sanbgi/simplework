#include "../inc/math/math.h"

using namespace sw::core;
using namespace sw::math;

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CTensor : public CObject, ITensor {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://ITensor
    int initVector(Data::DataType eDt, int nSize, void* pData) {

        release();

        int nElementByte = s_getDtSize(eDt);
        if(nSize <= 0 || nElementByte == 0 ) {
            return Error::ERRORTYPE_FAILURE;
        }

        int nDataSize = nSize*nElementByte;
        m_pElementData = new unsigned char[nDataSize+nElementByte];
        if(pData) {
            memcpy(m_pElementData, pData, nDataSize);
            memset(m_pElementData+nDataSize, 0, nElementByte);
        }else{
            memset(m_pElementData, 0, nDataSize+nElementByte);
        }

        m_nElementSize = nSize;
        m_nElementByte = nElementByte;
        return Error::ERRORTYPE_SUCCESS;
    }

    int initTensor( const Tensor& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData = nullptr) {

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
        
        if( initVector(eElementType, nElementSize, pElementData) != Error::ERRORTYPE_SUCCESS ) {
            return Error::ERRORTYPE_FAILURE;
        }
        m_spDimVector = spDimVector;
    }

    const Tensor& getDimVector() {
        if(!m_spDimVector && m_nElementSize > 0) {
            m_spDimVector = Tensor::createVector(1, &m_nElementSize);
        }
        return m_spDimVector;
    }

public:
    static int s_getDtSize(Data::DataType eDt) {
        switch (eDt)
        {
        case Data::DATATYPE_BOOL:
            return sizeof(bool);

        case Data::DATATYPE_CHAR:
            return sizeof(char);

        case Data::DATATYPE_SHORT:
            return sizeof(short);

        case Data::DATATYPE_INT:
            return sizeof(int);

        case Data::DATATYPE_LONG:
            return sizeof(long);

        case Data::DATATYPE_FLOAT:
            return sizeof(float);

        case Data::DATATYPE_DOUBLE:
            return sizeof(double);

        default:
            return 0;
        }
    }

public://ITensor

    Data::DataType getDataType() const {
        return m_eElementDt;
    }

    int getDataSize() const {
        return m_nElementSize;
    }

    const void* getDataPtr(Data::DataType eElementType, int iPos=0) const {
        if( eElementType == m_eElementDt &&
            iPos >=0 && iPos < m_nElementSize ) {
            return m_pElementData+iPos*m_nElementByte;
        }
        return nullptr;
    }

public:
    CTensor() {
        m_eElementDt = Data::DATATYPE_UNKNOWN;
        m_pElementData = nullptr;
        m_nElementSize = 0;
    }
    ~CTensor() {
        release();
    }
    void release() {
        if(m_pElementData) {
            delete [] m_pElementData;
            m_pElementData = nullptr;
        }
        m_eElementDt = Data::DATATYPE_UNKNOWN;
        m_nElementSize = 0;
    }

private:
    int m_nElementSize;
    int m_nElementByte;
    Data::DataType m_eElementDt;
    unsigned char* m_pElementData;
    Tensor m_spDimVector;
};

SIMPLEWORK_FACTORY_REGISTER(CTensor, "sw.math.Tensor")

SIMPLEWORK_MATH_NAMESPACE_LEAVE