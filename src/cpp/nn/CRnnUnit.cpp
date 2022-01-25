#include "CRnnUnit.h"
#include "CType.h"
#include "CUtils.h"
#include "CNnOperator.h"

static SCtx sCtx("CRnnUnit");
int CRnnUnit::createUnit(int nCells, const char* szActivator, SNnUnit& spUnit) {
    CPointer<CRnnUnit> spRnn;
    CObject::createObject(spRnn);
    spRnn->m_nCells = nCells;
    if( szActivator!=nullptr ){
        spRnn->m_strActivator = szActivator;
    }
    spUnit.setPtr(spRnn.getPtr());
    return sCtx.success();
}

int CRnnUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
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
    }

    SNnVariable x = spInVars[0];
    SNnVariable joinedx = SNnVariable::eval("join", x, m_spState);
    SNnVariable y = joinedx.product(m_spWeights);
    if(m_strActivator.length() > 0) {
        spOutVar = y.op(m_strActivator.c_str());
    }else{
        spOutVar = y.op("relu");
    }
    SNnVariable::eval("storeState", m_spState, y);
    return sCtx.success();
}

int CRnnUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visitString("activator", m_strActivator);

    //TODO

    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CRnnUnit, CRnnUnit::__getClassKey())