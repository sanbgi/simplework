#include "CPoolUnit.h"

static SCtx sCtx("CPoolUnit");
int CPoolUnit::createUnit(const PData& rData, SNnUnit& spUnit) {
    const PNnPool* pPool = CData<PNnPool>(rData);
    if(pPool == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    CPointer<CPoolUnit> spPool;
    CObject::createObject(spPool);
    spPool->m_nWidth = pPool->nWidth;
    spPool->m_nHeight = pPool->nHeight;
    spPool->m_nStrideWidth = pPool->nStrideWidth;
    spPool->m_nStrideHeight = pPool->nStrideHeight;
    spPool->m_dDropoutRate = 0;
    if( pPool->szPadding != nullptr) {
        spPool->m_strPaddingMode = pPool->szPadding;
    }
    spUnit.setPtr(spPool.getPtr());
    return sCtx.success();
}

int CPoolUnit::eval(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("池化单元输入参数必须为一个");
    }
    PNnPool poolParameter = { m_nWidth, m_nHeight, m_nStrideWidth, m_nStrideHeight };
    return SNnVariable::solveOp("pool", CData<PNnPool>(poolParameter), nInVars, pInVars, spOutVar);
}

int CPoolUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.visit("width", m_nWidth);
    ar.visit("height", m_nHeight);
    ar.visit("stridewidth", m_nStrideWidth);
    ar.visit("strideheight", m_nStrideHeight);
    ar.visit("dropoutRate", m_dDropoutRate);
    ar.visitString("padding", m_strPaddingMode);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CPoolUnit, CPoolUnit::__getClassKey())