
#include "module.h"

using namespace sw;
using namespace std;

//
//  标准化数据
//
static SCtx sCtx("CBatchNormalizeModule");
class CBatchNormalizeModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "BatchNormalizeModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnModule
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.BatchNormalizeModule"; }

private:
    //基础参数
    double m_dEsp;
    int m_nMinBatch;

public:
    CBatchNormalizeModule() {
        m_dEsp = 1.0e-8;
        m_nMinBatch = 10;
    }
};

int CBatchNormalizeModule::__initialize(const PData* pData){
    const PNnBatchNormalize* pBatchNormalize = CData<PNnBatchNormalize>(pData);
    if(pBatchNormalize == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_dEsp = pBatchNormalize->dEsp;
    m_nMinBatch = pBatchNormalize->nMinBatch;
    return sCtx.success();
}

int CBatchNormalizeModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("全连接网络输入参数必须为一个");
    }

    SNnVariable x = spInVars[0];
    spOutVar = x.batchNormalize({1.0e-8, m_nMinBatch});
    return sCtx.success();
}

int CBatchNormalizeModule::toArchive(const SArchive& ar) {
    ar.arBlock("esp", m_dEsp);
    ar.arBlock("minbatch", m_nMinBatch);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CBatchNormalizeModule, CBatchNormalizeModule::__getClassKey())