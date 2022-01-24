#ifndef __SimpleWork_NN_CRnnUnit_H__
#define __SimpleWork_NN_CRnnUnit_H__

#include "nn.h"
#include <string>

using namespace sw;
using namespace std;

class CRnnUnit : public CObject, public INnUnit, public IIoArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "RnnUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.RnnUnit"; }
    static int createUnit(int nCells, bool isBatchMode, const char* szActivator, SNnUnit& spUnit);

private:
    //基础参数
    int m_nCells;
    bool m_isBatchMode;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnVariable m_spState;

public:
    CRnnUnit() {
    }
};

#endif//__SimpleWork_NN_CRnnUnit_H__