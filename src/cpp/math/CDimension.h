
#ifndef __SimpleWork_Math_CDimension_H__
#define __SimpleWork_Math_CDimension_H__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CDimension : public CObject, public IDimension, IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDimension)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    static int createDimension(SDimension& spDim, int nDims, const int* pDimSizes);

public:
    int getClassVer() { return 220112; }
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.math.Dimension"; }

public://IDimension
    int getSize();
    const int* getData();
    int getElementSize();
    
public:
    int m_nElementSize;
    CTaker<int*> m_spData;
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_CDimension_H__