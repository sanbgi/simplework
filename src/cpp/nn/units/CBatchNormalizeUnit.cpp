
#include "unit.h"
#include "../solvers/PNnBatchNormalizeOperator.h"
#include <string>

using namespace sw;
using namespace std;

//
//  标准化数据
//
static SCtx sCtx("CBatchNormalizeUnit");
class CBatchNormalizeUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "BatchNormalizeUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.BatchNormalizeUnit"; }
    static int createUnit(const PData& rData, SNnUnit& spUnit);

private:
    //基础参数
    double m_dEsp;
    int m_nMinBatch;
    STensor m_spAvg;
    STensor m_spVariance;
    SNnVariable m_spGama;
    SNnVariable m_spBeta;

public:
    CBatchNormalizeUnit() {
        m_dEsp = 1.0e-8;
        m_nMinBatch = 10;
    }
};

int CBatchNormalizeUnit::__initialize(const PData* pData){
    const PNnBatchNormalize* pBatchNormalize = CData<PNnBatchNormalize>(pData);
    if(pBatchNormalize == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_dEsp = pBatchNormalize->dEsp;
    m_nMinBatch = 10;
    return sCtx.success();
}

int CBatchNormalizeUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("全连接网络输入参数必须为一个");
    }

    SNnVariable x = spInVars[0];
    PNnBatchNormalizeOperator parameter;
    parameter.dEsp = m_dEsp;
    parameter.nMinBatchs = m_nMinBatch;
    parameter.pAvg = &m_spAvg;
    parameter.pVariance = &m_spVariance;
    return SNnVariable::solveOp("batchnormalize", CData<PNnBatchNormalizeOperator>(parameter), 1, &x, spOutVar);
}

int CBatchNormalizeUnit::toArchive(const SArchive& ar) {
    ar.arObject("avg", m_spAvg);
    ar.arObject("variance", m_spVariance);
    ar.arObject("gama", m_spGama);
    ar.arObject("beta", m_spBeta);
    ar.arBlock("esp", m_dEsp);
    ar.arBlock("minbatch", m_nMinBatch);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CBatchNormalizeUnit, CBatchNormalizeUnit::__getClassKey())