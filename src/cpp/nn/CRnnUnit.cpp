#include "CRnnUnit.h"
#include "CType.h"
#include "CUtils.h"
#include "CNnOperator.h"

static SCtx sCtx("CRnnUnit");
int CRnnUnit::createUnit(int nCells, bool isBatchMode, const char* szActivator, SNnUnit& spUnit) {
    CPointer<CRnnUnit> spRnn;
    CObject::createObject(spRnn);
    spRnn->m_nCells = nCells;
    spRnn->m_isBatchMode = isBatchMode;
    if( szActivator!=nullptr ){
        spRnn->m_strActivator = szActivator;
    }
    spUnit.setPtr(spRnn.getPtr());
    return sCtx.success();
}

int CRnnUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("卷积单元输入参数必须为一个");
    }

    if(!m_spWeights) {
        /*
        SDimension spInDim = spInVars[0].dimension();

        int nDims = spInDim.size();
        const int* pDimSizes = spInDim.data();
        if(nDims < 2) {
            return sCtx.error("卷积的输入张量维度至少要大于2");
        }

        int nLayers = 1;
        for(int i=2; i<nDims; i++) {
            nLayers *= pDimSizes[i];
        }

        int pWeightDimSizes[5] = { m_nLayers, m_nShiftConvs, m_nHeight, m_nWidth, nLayers };
        if( SNnVariable::createWeight(5, pWeightDimSizes, m_spWeights) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }

        if( SNnVariable::createWeight(1, &m_nLayers, m_spBais) != sCtx.success() ) {
            return sCtx.error("偏置创建失败");
        }*/
    }

    SNnVariable y;

    if(m_strActivator.length() > 0) {
        spOutVar = y.op(m_strActivator.c_str());
    }else{
        spOutVar = y.op("relu");
    }
    return sCtx.success();
}

int CRnnUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visitString("activator", m_strActivator);

    //TODO

    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CRnnUnit, CRnnUnit::__getClassKey())