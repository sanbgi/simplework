
#include "module.h"
#include <string>

using namespace sw;
using namespace std;
static SCtx sCtx("CConvModule");
class CConvModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "ConvModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnModule
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ConvModule"; }

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

public:
    CConvModule() {
    }
};

int CConvModule::__initialize(const PData* pData) {
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

int CConvModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("卷积单元输入参数必须为一个");
    }

    spOutVar = spInVars[0].conv({m_nWidth, m_nHeight, m_nLayers, m_nShiftConvs, m_nStrideWidth, m_nStrideHeight, m_strPaddingMode.c_str(), m_strActivator.c_str()});
    return sCtx.success();
}

int CConvModule::toArchive(const SArchive& ar) {
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
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CConvModule, CConvModule::__getClassKey())