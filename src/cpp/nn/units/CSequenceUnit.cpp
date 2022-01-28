#include "CSequenceUnit.h"

static SCtx sCtx("CSequenceUnit");
int CSequenceUnit::createUnit(int nInUnits, const SNnUnit pInUnits[], SNnUnit& spUnit) {
    CPointer<CSequenceUnit> spSequence;
    CObject::createObject(spSequence);
    for(int i=0; i<nInUnits; i++) {
        spSequence->m_arrUnits.push_back(pInUnits[i]);
    }
    spUnit.setPtr(spSequence.getPtr());
    return sCtx.success();
}

int CSequenceUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("序列执行单元输入参数必须为一个");
    }

    SNnVariable x = spInVars[0];
    vector<SNnUnit>::iterator it = m_arrUnits.begin();
    while(it != m_arrUnits.end()) {
        SNnVariable y;
        if( (*it)->eval(1, &x, y) != sCtx.success() ){
            return sCtx.error("计算错误");
        }
        x = y;
        it++;
    }
    spOutVar = x;
    return sCtx.success();
}

int CSequenceUnit::toArchive(const SArchive& ar) {
    ar.visitObjectArray("units", m_arrUnits);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CSequenceUnit, CSequenceUnit::__getClassKey())