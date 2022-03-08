#include "module.h"
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CPoolModule");
class CPoolModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "PoolModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnModule
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.PoolModule"; }

private:
    //基础参数
    int m_nWidth;
    int m_nHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;
    double m_dDropoutRate;
    string m_strPaddingMode;

public:
    CPoolModule() {
    }
};

int CPoolModule::__initialize(const PData* pData) {
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

int CPoolModule::eval(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("池化单元输入参数必须为一个");
    }
    spOutVar = pInVars[0].maxpool({ m_nWidth, m_nHeight, m_nStrideWidth, m_nStrideHeight, m_strPaddingMode.c_str() });
    return sCtx.success();
}

int CPoolModule::toArchive(const SArchive& ar) {
    //基础参数
    ar.arBlock("width", m_nWidth);
    ar.arBlock("height", m_nHeight);
    ar.arBlock("stridewidth", m_nStrideWidth);
    ar.arBlock("strideheight", m_nStrideHeight);
    ar.arBlock("dropoutRate", m_dDropoutRate);
    ar.visitString("padding", m_strPaddingMode);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CPoolModule, CPoolModule::__getClassKey())