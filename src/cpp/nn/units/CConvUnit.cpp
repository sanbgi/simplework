#include "CConvUnit.h"

static SCtx sCtx("CConvUnit");
int CConvUnit::createUnit(int nWidth, int nHeight, int nLayers, int nShiftConvs, const char* szPaddingMode, const char* szActivator, SNnUnit& spUnit) {
    CPointer<CConvUnit> spConv;
    CObject::createObject(spConv);
    spConv->m_nWidth = nWidth;
    spConv->m_nHeight = nHeight;
    spConv->m_nLayers = nLayers;
    spConv->m_nShiftConvs = nShiftConvs;
    spConv->m_dDropoutRate = 0;
    if( szPaddingMode != nullptr) {
        spConv->m_strPaddingMode = szPaddingMode;
    }
    if( szActivator!=nullptr ){
        spConv->m_strActivator = szActivator;
    }
    spUnit.setPtr(spConv.getPtr());
    return sCtx.success();
}

int CConvUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("卷积单元输入参数必须为一个");
    }

    if(!m_spWeights) {
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
        if( SNnVariable::createWeight(SDimension(5, pWeightDimSizes), m_spWeights) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }

        if( SNnVariable::createWeight(SDimension(1, &m_nLayers), m_spBais) != sCtx.success() ) {
            return sCtx.error("偏置创建失败");
        }
    }

    SNnVariable y;
    SNnVariable inConv[3] = { spInVars[0], m_spWeights, m_spBais };

    PNnConv convData;
    convData.szPadding = m_strPaddingMode.c_str();
    if( SNnVariable::solveOp("conv", CData<PNnConv>(convData), 3, inConv, y ) != sCtx.success() ) {
        return sCtx.error("卷积运算错误");
    }

    if(m_strActivator.length() > 0) {
        spOutVar = y.solveOp(m_strActivator.c_str());
    }else{
        spOutVar = y.solveOp("relu");
    }
    return sCtx.success();
}

int CConvUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.visit("width", m_nWidth);
    ar.visit("height", m_nHeight);
    ar.visit("layers", m_nLayers);
    ar.visit("shiftconvs", m_nShiftConvs);
    ar.visitString("padding", m_strPaddingMode);
    ar.visitString("activator", m_strActivator);
    ar.visit("dropoutRate", m_dDropoutRate);
    ar.visitObject("weight", m_spWeights);
    ar.visitObject("bais", m_spBais);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CConvUnit, CConvUnit::__getClassKey())