
#include "unit.h"
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CRnnUnit");
class CRnnUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);
    
private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "RnnUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.RnnUnit"; }

private:
    //基础参数
    int m_nCells;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnState m_spState;

public:
    CRnnUnit() {
    }
};

int CRnnUnit::__initialize(const PData* pData) {
    const PNnRnn* pRnn = CData<PNnRnn>(pData);
    if(pRnn == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_nCells = pRnn->nCells;
    if( pRnn->szActivator!=nullptr ){
        m_strActivator = pRnn->szActivator;
    }
    return sCtx.success();
}

int CRnnUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("Rnn单元输入参数必须为一个");
    }

    if(!m_spWeights) {
        int nInputSize = spInVars[0].dimension().dataSize();
        if(nInputSize < 1) {
            return sCtx.error("Rnn单元的输入必须大于等于1");
        }

        if( SNnVariable::createState(SDimension(1, &m_nCells), m_spState) != sCtx.success() ) {
            return sCtx.error("偏置状态失败");
        }

        int pDimSizes[2] = {m_nCells, m_nCells+nInputSize };
        if( SNnVariable::createWeight(SDimension(2, pDimSizes), m_spWeights) != sCtx.success() ) {
            return sCtx.error("权重变量创建失败");
        }
    }

    SNnVariable state = SNnVariable::loadState(m_spState);
    SNnVariable joinedx = SNnVariable::solveOp("join", state, spInVars[0]);
    SNnVariable y = SNnVariable::product(joinedx,m_spWeights);
    if(m_strActivator.length() > 0) {
        spOutVar = y.solveOp(m_strActivator.c_str());
    }else{
        spOutVar = y.solveOp("relu");
    }
    SNnVariable::saveState(m_spState, spOutVar);
    return sCtx.success();
}

int CRnnUnit::toArchive(const SArchive& ar) {
    ar.visit("cells", m_nCells);
    ar.visitString("activator", m_strActivator);
    ar.visitObject("weight", m_spWeights);
    ar.visitObject("state", m_spState);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CRnnUnit, CRnnUnit::__getClassKey())