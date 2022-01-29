#include "CPoolUnit.h"

static SCtx sCtx("CPoolUnit");
int CPoolUnit::__initialize(const PData* pData) {
    const PNnPool* pPool = CData<PNnPool>(pData);
    if(pPool == nullptr) {
        return sCtx.error("缺少构造参数");
    }

    m_nWidth = pPool->nWidth;
    m_nHeight = pPool->nHeight;
    m_nStrideWidth = pPool->nStrideWidth;
    m_nStrideHeight = pPool->nStrideHeight;
    m_dDropoutRate = 0;
    if( pPool->szPadding != nullptr) {
        m_strPaddingMode = pPool->szPadding;
    }
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