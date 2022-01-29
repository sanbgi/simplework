#ifndef __SimpleWork_NN_CPoolUnit_H__
#define __SimpleWork_NN_CPoolUnit_H__

#include "unit.h"
#include <string>

using namespace sw;
using namespace std;

class CPoolUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "PoolUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.PoolUnit"; }

private:
    //基础参数
    int m_nWidth;
    int m_nHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dDropoutRate;
    string m_strPaddingMode;

public:
    CPoolUnit() {
    }
};

#endif//__SimpleWork_NN_CPoolUnit_H__
