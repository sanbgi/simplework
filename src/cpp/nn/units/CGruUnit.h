#ifndef __SimpleWork_NN_CGruUnit_H__
#define __SimpleWork_NN_CGruUnit_H__

#include "unit.h"
#include <string>

using namespace sw;
using namespace std;

class CGruUnit : public CObject, public INnUnit, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "GruUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.GruUnit"; }
    static int createUnit(int nCells, SNnUnit& spUnit);

private:
    //基础参数
    int m_nCells;
    SNnVariable m_spWeights;
    SNnVariable m_spWeightsZ;
    SNnVariable m_spWeightsR;
    SNnVariable m_spState;

public:
    CGruUnit() {
    }
};

#endif//__SimpleWork_NN_CGruUnit_H__
