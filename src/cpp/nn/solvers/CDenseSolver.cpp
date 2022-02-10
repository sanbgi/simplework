
#include "operator.h"
#include <string>

using namespace sw;
using namespace std;


//
//  全连接神经网络，注意数据格式：
//
//      输入数据：nBatchs(第一个维度) * nCells(余下所有维度)
//      输出数据：nBatchs(第一个维度) * nCells(第二个维度)
//
//  比如：输入维度[10, 3, 1]，表示一共有十个输入数据，每一个数据数据包含3 X 1个输入神经元
//
static SCtx sCtx("CDenseSolver");
class CDenseSolver : public CObject, public INnSolver, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "DenseSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        return sCtx.success();
    }

private://INnUnit
    int solve(const PData* pData, int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
        const PNnDense* pDense = CData<PNnDense>(pData);
        if(pDense == nullptr) {
            return sCtx.error("缺少参数");
        }

        if(nInVars != 1) {
            return sCtx.error("全连接网络输入参数必须为一个");
        }

        SDimension spDim = spInVars[0].dimension();
        int nInputCells = spDim->getElementSize();
        int pWeightDimSizes[2] = {pDense->nCells, nInputCells};
        SNnVariable spWeights = SNnVariable::createWeight({SDimension(2, pWeightDimSizes), 1.0f/nInputCells});
        SNnVariable spBais = SNnVariable::createWeight({SDimension(1, &pDense->nCells),0});
        if( !spWeights || !spBais ) {
            return sCtx.error("权重变量创建失败");
        }
    
        SNnVariable x = spInVars[0];
        SNnVariable y = SNnVariable::product(x, spWeights) + spBais;
        if(pDense->szActivator != nullptr && pDense->szActivator[0] != 0 ) {
            spOutVar = y.solveOp(pDense->szActivator);
        }else{
            spOutVar = y;
        }
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.DenseSolver"; }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CDenseSolver, CDenseSolver::__getClassKey())
static SNnSolverRegister s_Register("dense", CNnSolver::createStaticSolver<CDenseSolver>);
