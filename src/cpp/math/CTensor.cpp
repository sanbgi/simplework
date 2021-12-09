#include "../inc/math/math.h"

using namespace sw::core;
using namespace sw::math;

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CTensor : public CObject, ITensor {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
        SIMPLEWORK_INTERFACE_ENTRY(IVector)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://ITensor
    int initTensor( const Vector& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData = nullptr) {
        Vector spDataVector = Vector::createVector();
        if( spDataVector->initVector(eElementType, nElementSize, pElementData) != Error::ERRORTYPE_SUCCESS) {
            return Error::ERRORTYPE_FAILURE;
        }
        return initTensor(spDimVector, spDataVector);
    }

    int initTensor( const Vector& spDimVector, const Vector& spDataVector) {
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
        if(nSize!= spDataVector->getDataSize()) {
            return Error::ERRORTYPE_FAILURE;
        }
        m_spDataVector = spDataVector;
        m_spDimVector = spDimVector;
        return Error::ERRORTYPE_SUCCESS;
    }

public://IVector
    int initVector(Data::DataType eElementType, int nElementSize, void* pElementData = nullptr) {
        Vector spDimVector = Vector::createVector(1, &nElementSize);
        return initTensor(spDimVector, eElementType, nElementSize, pElementData);
    }

    Data::DataType getDataType() const {
        return m_spDataVector ? m_spDataVector->getDataType() : Data::DATATYPE_UNKNOWN;
    }

    int getDataSize() const {
        return m_spDataVector ? m_spDataVector->getDataSize() : 0;
    }

    const void* getDataPtr(Data::DataType eElementType, int iPos=0) const {
        return m_spDataVector ? m_spDataVector->getDataPtr(eElementType, iPos) : nullptr;
    }

    const Vector& getDataVector() {
        return m_spDataVector;
    }

    const Vector& getDimVector() {
        return m_spDimVector;
    }

private:
    Vector m_spDataVector;
    Vector m_spDimVector;
};

SIMPLEWORK_FACTORY_REGISTER(CTensor, "sw.math.Tensor")

SIMPLEWORK_MATH_NAMESPACE_LEAVE