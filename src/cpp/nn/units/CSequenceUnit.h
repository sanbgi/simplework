#ifndef __SimpleWork_NN_CSequenceUnit_H__
#define __SimpleWork_NN_CSequenceUnit_H__

#include "unit.h"
#include <vector>

using namespace sw;
using namespace std;

class CSequenceUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "SequenceUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.SequenceUnit"; }
    static int createUnit(int nInUnits, const SNnUnit pInUnits[], SNnUnit& spUnit);

private:
    vector<SNnUnit> m_arrUnits;
    
public:
    CSequenceUnit() {
    }
};

#endif//__SimpleWork_NN_CSequenceUnit_H__
