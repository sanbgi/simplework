#include "operator.h"

using namespace sw;
using namespace std;
static SCtx sCtx("CGruSolver");
class CGruSolver : public CObject, public INnSolver, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INnUnit
    int solve(const PData* pData, int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "GruSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.GruSolver"; }
};

int CGruSolver::solve(const PData* pData, int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
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

    SNnState spState;
    if( SNnVariable::createState(SDimension(1, &pRnn->nCells), spState) != sCtx.success() ) {
        return sCtx.error("偏置状态失败");
    }

    int nJoinedSize = pRnn->nCells+nInputSize;
    int pDimSizes[2] = {pRnn->nCells, nJoinedSize };
    SDimension spDim(2, pDimSizes);
    SNnVariable spWeights = SNnVariable::createWeight({spDim, 1.0f/nJoinedSize});
    if( !spWeights ) {
        return sCtx.error("权重变量创建失败");
    }

    SNnVariable spWeightsZ = SNnVariable::createWeight({spDim, 1.0f/nJoinedSize});
    if( !spWeightsZ ) {
        return sCtx.error("权重变量创建失败");
    }

    SNnVariable spWeightsR = SNnVariable::createWeight({spDim, 1.0f/nJoinedSize});
    if( !spWeightsR ) {
        return sCtx.error("权重变量创建失败");
    }
    
    SNnVariable state = SNnVariable::loadState(spState);
    SNnVariable joinedx = SNnVariable::solveOp("join", state, spInVars[0] );
    SNnVariable z = SNnVariable::product(joinedx, spWeightsZ).solveOp("sigmoid");
    SNnVariable r = SNnVariable::product(joinedx, spWeightsR).solveOp("sigmoid");
    SNnVariable hh = SNnVariable::solveOp("join", r * state, spInVars[0]);
    hh = SNnVariable::product(hh,spWeights).solveOp("tanh");
    spOutVar = hh - z * state + z * hh; 
    SNnVariable::saveState(spState, spOutVar);
    return sCtx.success();
}

int CGruSolver::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CGruSolver, CGruSolver::__getClassKey())
static SNnSolverRegister s_Register("gru", CNnSolver::createStaticSolver<CGruSolver>);
