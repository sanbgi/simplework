#include "CDenseUnit.h"

static SCtx sCtx("CDenseUnit");
int CDenseUnit::createUnit(int nCells, double dDropoutRate, const char* szActivator, SNnUnit& spUnit) {
    CPointer<CDenseUnit> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    spDense->m_dDropoutRate = dDropoutRate;
    if( szActivator!=nullptr )
        spDense->m_strActivator = szActivator;
    spUnit.setPtr(spDense.getPtr());
    return sCtx.success();
}

int CDenseUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("全连接网络输入参数必须为一个");
    }

    if(!m_spWeights) {
        SDimension spDim = spInVars[0].dimension();

        int pWeightDimSizes[2] = {m_nCells, spDim->getElementSize()};
        if( SNnVariable::createWeight(2, pWeightDimSizes, m_spWeights) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }

        if( SNnVariable::createWeight(1, &m_nCells, m_spBais) != sCtx.success() ) {
            return sCtx.error("偏置创建失败");
        }
    }

    SNnVariable x = spInVars[0];
    SNnVariable y = x.product(m_spWeights) + m_spBais;
    if(m_strActivator.length() > 0) {
        spOutVar = y.op(m_strActivator.c_str());
    }else{
        spOutVar = y.op("relu");
    }
    return sCtx.success();
}

int CDenseUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visit("dropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    ar.visitObject("weight", m_spWeights);
    ar.visitObject("bais", m_spBais);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDenseUnit, CDenseUnit::__getClassKey())