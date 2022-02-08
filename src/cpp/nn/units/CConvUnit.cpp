
#include "unit.h"
#include <string>

using namespace sw;
using namespace std;
static SCtx sCtx("CConvUnit");
class CConvUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "ConvUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ConvUnit"; }

private:
    //基础参数
    int m_nWidth;
    int m_nHeight;
    int m_nLayers;
    int m_nShiftConvs;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dDropoutRate;
    string m_strPaddingMode;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnVariable m_spBais;

public:
    CConvUnit() {
    }
};

int CConvUnit::__initialize(const PData* pData) {
    const PNnConv* pConv = CData<PNnConv>(pData);
    if(pConv == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_nWidth = pConv->nWidth;
    m_nHeight = pConv->nHeight;
    m_nLayers = pConv->nLayers;
    m_nShiftConvs = pConv->nShiftConvs;
    m_nStrideWidth = pConv->nStrideWidth;
    m_nStrideHeight = pConv->nStrideHeight;
    m_dDropoutRate = 0;
    if( pConv->szPadding != nullptr) {
        m_strPaddingMode = pConv->szPadding;
    }
    if( pConv->szActivator!=nullptr ){
        m_strActivator = pConv->szActivator;
    }
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
        m_spWeights = SNnVariable::createWeight({SDimension(5, pWeightDimSizes), 0.5});
        m_spBais = SNnVariable::createWeight({SDimension(1, &m_nLayers), 0});
        if( !m_spWeights || !m_spBais ) {
            return sCtx.error("权重创建失败");
        }
    }

    SNnVariable y;
    SNnVariable inConv[3] = { spInVars[0], m_spWeights, m_spBais };

    PNnConv convData;
    convData.szPadding = m_strPaddingMode.c_str();
    convData.nStrideHeight = m_nStrideHeight;
    convData.nStrideWidth = m_nStrideWidth;
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
    ar.arBlock("width", m_nWidth);
    ar.arBlock("height", m_nHeight);
    ar.arBlock("layers", m_nLayers);
    ar.arBlock("shiftconvs", m_nShiftConvs);
    ar.arBlock("stridewidth", m_nStrideWidth);
    ar.arBlock("strideheight", m_nStrideHeight);
    ar.visitString("padding", m_strPaddingMode);
    ar.visitString("activator", m_strActivator);
    ar.arBlock("dropoutRate", m_dDropoutRate);
    ar.arObject("weight", m_spWeights);
    ar.arObject("bais", m_spBais);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CConvUnit, CConvUnit::__getClassKey())