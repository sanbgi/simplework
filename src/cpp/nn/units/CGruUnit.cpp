#include "CGruUnit.h"

static SCtx sCtx("CGruUnit");
int CGruUnit::createUnit(int nCells, SNnUnit& spUnit) {
    CPointer<CGruUnit> spRnn;
    CObject::createObject(spRnn);
    spRnn->m_nCells = nCells;
    spUnit.setPtr(spRnn.getPtr());
    return sCtx.success();
}

int CGruUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("Rnn单元输入参数必须为一个");
    }

    if(!m_spWeights) {
        int nInputSize = spInVars[0].dimension().dataSize();
        if(nInputSize < 1) {
            return sCtx.error("Rnn单元的输入必须大于等于1");
        }

        if( SNnVariable::createState(SDimension(1, &m_nCells), m_spState) != sCtx.success() ) {
            return sCtx.error("偏置状态失败");
        }

        int pDimSizes[2] = {m_nCells, m_nCells+nInputSize };
        if( SNnVariable::createWeight(SDimension(2, pDimSizes), m_spWeights) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }

        if( SNnVariable::createWeight(SDimension(2, pDimSizes), m_spWeightsZ) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }

        if( SNnVariable::createWeight(SDimension(2, pDimSizes), m_spWeightsR) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }
    }

    SNnVariable state = SNnVariable::loadState(m_spState);
    SNnVariable joinedx = SNnVariable::eval("join", state, spInVars[0] );
    SNnVariable z = SNnVariable::product(joinedx, m_spWeightsZ).op("sigmod");
    SNnVariable r = SNnVariable::product(joinedx, m_spWeightsR).op("sigmod");
    SNnVariable hh = SNnVariable::eval("join", r * state, spInVars[0]);
    hh = SNnVariable::product(hh,m_spWeights).op("tanh");
    spOutVar = hh - z * state + z * hh; 
    SNnVariable::saveState(m_spState, spOutVar);
    return sCtx.success();
}

int CGruUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visitObject("weight", m_spWeights);
    ar.visitObject("weightz", m_spWeightsZ);
    ar.visitObject("weightr", m_spWeightsR);
    ar.visitObject("state", m_spState);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGruUnit, CGruUnit::__getClassKey())