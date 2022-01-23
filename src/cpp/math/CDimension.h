
#ifndef __SimpleWork_Math_CDimension_H__
#define __SimpleWork_Math_CDimension_H__

#include "math.h"
#include "CTensor.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CDimension : public CTensor, public IDimension {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CTensor)
        SIMPLEWORK_INTERFACE_ENTRY(IDimension)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CTensor)

public:
    int getClassVer() { return 220112; }
    const char* getClassName() { return "Dimension"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar) {
        return CTensor::toArchive(ar);
    }

public://Factory
    static const char* __getClassKey() { return "sw.math.Dimension"; }

public://IDimension
    int getSize();
    const int* getData();
    int getElementSize();
    int getVector(STensor& spDimVector);
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_CDimension_H__