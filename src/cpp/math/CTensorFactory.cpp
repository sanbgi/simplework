#include "CTensorFactory.h"

using namespace sw;
using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensor");

//
// 张量模板类
//
template<typename T> class CTensor : public CObject, public ITensor {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://ITensor
    int initVector(int nSize, void* pData) {

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

    int initTensor( const SDimension& spDimVector, int nElementSize, void* pElementData = nullptr) {
        if( spDimVector ) {
            if(nElementSize!= spDimVector->getElementSize()) {
                return SError::ERRORTYPE_FAILURE;
            }
        }
        
        if( initVector(nElementSize, pElementData) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        m_spDimVector = spDimVector;
        return SError::ERRORTYPE_SUCCESS;
    }

    SDimension& getDimVector() {
        if(!m_spDimVector && m_nElementSize > 0) {
            SDimension::createDimension(m_spDimVector, 1, &m_nElementSize);
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

    int getDimension(SDimension& spDim) {
        spDim = getDimVector();
        return sCtx.success();
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
    static int createTensor(STensor& spTensor, const SDimension* pDimension, int nElementSize, void* pElementData) {
        CPointer<CTensor<T>> sp;
        CObject::createObject(sp);
        if(pDimension) {
            if( sp->initTensor(*pDimension, nElementSize, pElementData) != sCtx.success() ) {
                return sCtx.error("初始化张量失败");
            }
        }else{
            if( sp->initVector(nElementSize, pElementData) != sCtx.success() ) {
                return sCtx.error("初始化张量失败");
            }
        }
        spTensor.setPtr(sp.getPtr());
        return sCtx.success();
    }

protected:
    int m_nVer;
    int m_nElementSize;
    CTaker<T*> m_spElementData;
    SDimension m_spDimVector;
};

class CDimension : public CTensor<int>, public IDimension {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CTensor)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CTensor)

public://IDimension
    int getSize() {
        return m_nElementSize;
    }

    const int* getData() {
        return (const int*)getDataPtr(CBasicData<int>::getStaticType());
    }

    int getElementSize() {
        const int* pDimSizes = getData();
        if(pDimSizes == nullptr) {
            return sCtx.error("当前向量并非维度向量");
        }
        int nElementSize = 1;
        for(int i=0; i<m_nElementSize; i++) {
            nElementSize *= pDimSizes[i];
        }
        return nElementSize;
    }

    int getVector(STensor& spDimVector) {
        spDimVector.setPtr((ITensor*)this);
        return sCtx.success();
    }

public:
    static int createDimension(SDimension& spDim, int nDims, const int* pDimSizes) {
        CPointer<CDimension> spTensor;
        CObject::createObject(spTensor);
        if( spTensor->initVector(nDims, (void*)pDimSizes) != sCtx.success()) {
            return sCtx.error("创建维度失败");
        }
        spDim.setPtr(spTensor.getPtr());
        return sCtx.success();
    }
};

typedef int (*FCreateTensor)(STensor& spTensor, const SDimension* pDimension, int nElementSize, void* pElementData);
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

int CTensorFactory::createVector( STensor& spTensor, unsigned int idElementType, int nElementSize, void* pElementData) {
    std::map<unsigned int, FCreateTensor>::iterator it = g_tensorFacotries.find(idElementType);
    if( it != g_tensorFacotries.end() ) {
        return (*(it->second))(spTensor, nullptr, nElementSize, pElementData);
    }
    return sCtx.error("无法创建指定类型的张量");
}

int CTensorFactory::createTensor( STensor& spTensor, const SDimension& spDimVector, unsigned int idElementType, int nElementSize, void* pElementData){
    std::map<unsigned int, FCreateTensor>::iterator it = g_tensorFacotries.find(idElementType);
    if( it != g_tensorFacotries.end() ) {
        return (*(it->second))(spTensor, &spDimVector, nElementSize, pElementData);
    }
    return sCtx.error("无法创建指定类型的张量");
}

int CTensorFactory::createDimension(SDimension& spDimension, int nElementSize, const int* pElementData) {
    return CDimension::createDimension(spDimension, nElementSize, pElementData);
}

SIMPLEWORK_MATH_NAMESPACE_LEAVE