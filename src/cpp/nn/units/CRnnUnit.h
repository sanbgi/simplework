#ifndef __SimpleWork_NN_CRnnUnit_H__
#define __SimpleWork_NN_CRnnUnit_H__

#include "unit.h"
#include <string>

using namespace sw;
using namespace std;

class CRnnUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)


private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);
    
private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "RnnUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.RnnUnit"; }
    static int createUnit(int nCells, const char* szActivator, SNnUnit& spUnit);

private:
    //基础参数
    int m_nCells;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnState m_spState;

public:
    CRnnUnit() {
    }
};

#endif//__SimpleWork_NN_CRnnUnit_H__
