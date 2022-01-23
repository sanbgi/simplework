#include "CPoolUnit.h"
#include "CType.h"
#include "CUtils.h"
#include "CNnOperator.h"

static SCtx sCtx("CPoolUnit");
int CPoolUnit::createUnit(int nWidth, int nHeight, int nStride, const char* szPaddingMode, SNnUnit& spUnit) {
    CPointer<CPoolUnit> spPool;
    CObject::createObject(spPool);
    spPool->m_nWidth = nWidth;
    spPool->m_nHeight = nHeight;
    spPool->m_nStride = nStride;
    spPool->m_dDropoutRate = 0;
    if( szPaddingMode != nullptr) {
        spPool->m_strPaddingMode = szPaddingMode;
    }
    spUnit.setPtr(spPool.getPtr());
    return sCtx.success();
}

int CPoolUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("池化单元输入参数必须为一个");
    }

    if( CNnOperator::createPoolVariable(m_strPaddingMode.c_str(), m_nWidth, m_nHeight, m_nStride, nInVars, spInVars, spOutVar ) != sCtx.success() ) {
        return sCtx.error("卷积运算错误");
    }
    return sCtx.success();
}

int CPoolUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("dropoutRate", m_dDropoutRate);

    //TODO

    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CPoolUnit, CPoolUnit::__getClassKey())