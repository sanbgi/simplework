
#include "operator.h"
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CRnnSolver");
class CRnnSolver : public CObject, public INnSolver, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnSolver
    int solve(const PData* pData, int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);
    
private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "RnnSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.RnnSolver"; }
};

int CRnnSolver::solve(const PData* pData, int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    const PNnRnn* pRnn = CData<PNnRnn>(pData);
    if(pRnn == nullptr) {
        return sCtx.error("缺少参数");
    }

    if(nInVars != 1) {
        return sCtx.error("Rnn单元输入参数必须为一个");
    }

    int nInputSize = spInVars[0].dimension().dataSize();
    if(nInputSize < 1) {
        return sCtx.error("Rnn单元的输入必须大于等于1");
    }

    SDimension spOutDim(1, &pRnn->nCells);
    SNnState spState;
    if( SNnVariable::createState(spOutDim, spState) != sCtx.success() ) {
        return sCtx.error("偏置状态失败");
    }

    int pDimSizes[2] = {pRnn->nCells, pRnn->nCells+nInputSize };
    SNnVariable spWeights = SNnVariable::createWeight({SDimension(2, pDimSizes), 1.0f/(pRnn->nCells+nInputSize)});
    SNnVariable spBais = SNnVariable::createWeight({SDimension(1, &pRnn->nCells), 0});
    if( !spWeights || !spWeights ) {
        return sCtx.error("权重变量创建失败");
    }
    
    SNnVariable state = SNnVariable::loadState(spState);
    SNnVariable joinedx = SNnVariable::solveOp("join", state, spInVars[0]);
    SNnVariable y = SNnVariable::product(joinedx,spWeights) + spBais;
    if(pRnn->szActivator != nullptr && pRnn->szActivator[0] != 0) {
        spOutVar = y.solveOp(pRnn->szActivator);
    }else{
        spOutVar = y.solveOp("relu");
    }
    SNnVariable::saveState(spState, spOutVar);
    return sCtx.success();
}

int CRnnSolver::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CRnnSolver, CRnnSolver::__getClassKey())
static SNnSolverRegister s_Register("rnn", CNnSolver::createStaticSolver<CRnnSolver>);
