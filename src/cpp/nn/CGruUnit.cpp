#include "CGruUnit.h"
#include "CType.h"
#include "CUtils.h"
#include "CNnOperator.h"

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

    SNnVariable joinedx = SNnVariable::eval("join", m_spState, spInVars[0] );
    SNnVariable z = joinedx.product(m_spWeightsZ).op("sigmod");
    SNnVariable r = joinedx.product(m_spWeightsR).op("sigmod");
    SNnVariable hh = SNnVariable::eval("join", r * m_spState, spInVars[0]);
    hh = hh.product(m_spWeights).op("tanh");
    spOutVar = hh - z * m_spState + z * hh; 
    SNnVariable::eval("storeState", m_spState, spOutVar);
    return sCtx.success();
}

int CGruUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visitObject("weight", m_spWeights);
    ar.visitObject("weightz", m_spWeightsZ);
    ar.visitObject("weightr", m_spWeightsR);
    ar.visitObject("state", m_spState);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGruUnit, CGruUnit::__getClassKey())