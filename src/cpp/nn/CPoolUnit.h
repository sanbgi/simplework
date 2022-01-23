#ifndef __SimpleWork_NN_CPoolUnit_H__
#define __SimpleWork_NN_CPoolUnit_H__

#include "nn.h"
#include <string>

using namespace sw;
using namespace std;

class CPoolUnit : public CObject, public INnUnit, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "PoolUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.PoolUnit"; }
    static int createUnit(int nWidth, int nHeight, int nStride, const char* szPaddingMode, SNnUnit& spUnit);

private:
    //基础参数
    int m_nWidth;
    int m_nHeight;
    int m_nStride;
    double m_dDropoutRate;
    string m_strPaddingMode;

public:
    CPoolUnit() {
    }
};

#endif//__SimpleWork_NN_CPoolUnit_H__
