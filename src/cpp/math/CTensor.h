
#ifndef __SimpleWork_Math_CTensorFactory_H__
#define __SimpleWork_Math_CTensorFactory_H__

#include "math.h"
#include <map>

using namespace sw;
using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CTypeAssist;
class CTensor : public CObject, public ITensor, public IIoArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "Tensor"; } 
    const char* getClassKey() { return __getClassKey(); }
    virtual int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.math.Tensor"; }

public://ITensor
    int initVector(CTypeAssist* pTypeAssist, int nSize, void* pData);
    int initTensor(CTypeAssist* pTypeAssist, const SDimension& spDimVector, int nElementSize, void* pElementData = nullptr);
    SDimension& getDimVector();

public://ITensor
    int getVer();
    int updateVer();
    int getDimension(SDimension& spDim);
    unsigned int getDataType();
    int getDataSize();
    void* getDataPtr(unsigned int idElementType, int iPos=0);

public:
    CTensor();
    ~CTensor();
    void release();

public:
    static int createTensor(STensor& spTensor, const SDimension* pDimension, unsigned int idElementType, int nElementSize, void* pElementData);
    static int createDimension(SDimension& spDimension, int nElementSize, const int* pElementData);

protected:
    CTypeAssist* m_pTypeAssist;
    int m_nVer;
    int m_nElementSize;
    CTaker<char*> m_spElementData;
    SDimension m_spDimVector;
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_CTensorFactory_H__