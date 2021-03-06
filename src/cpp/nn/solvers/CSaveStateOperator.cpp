#ifndef __SimpleWork_NN_Operators_CSaveStateOperator_h__
#define __SimpleWork_NN_Operators_CSaveStateOperator_h__

#include "operator.h"
static SCtx sCtx("CSaveStateOperator");

//
// 备份状态值到State中，并且在反向计算中，恢复偏导数
//
class CSaveStateOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size == inVars[1].size)
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn1End = pIn1 + inVars[0].size;
        Q* pIn2 = (Q*)inVars[1].data;
        while(pIn1 < pIn1End) {
            *pIn1 = *pIn2;
            pIn1++, pIn2++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size == inVars[1].size)
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia1End = pDevia1 + inVars[0].size;
        Q* pDevia2 = (Q*)inVars[1].devia;
        while(pDevia1 < pDevia1End) {
            *pDevia2 += *pDevia1;

            //
            //  反向求偏导时，由于每次单元计算都是最后才备份状态，所以，反向
            //计算时，应该第一步恢复偏导数，然后将状态的偏导数清零，准备计算下一步的偏导数.
            //  哈哈tricky?(和CNnVariableSolver对状态赋值部分的策略配合使用)
            //
            *pDevia1 = 0;
            pDevia1++, pDevia2++;
        }
    }

    int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) {
        if(solveCtx.idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameterData = this;
            return sCtx.success();
        }else if(solveCtx.idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameterData = this;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }
    
    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        if(nInVars != 2) {
            return sCtx.error("存储状态参数必须为两个，一个为状态变量，一个为计算变量");
        }
        return addAtomOperator(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getName() { return "SaveStateSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.SaveStateSolver"; }

};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CSaveStateOperator, CSaveStateOperator::__getClassKey())
static SNnSolverRegister s_Register("saveState", CNnSolver::createStaticSolver<CSaveStateOperator>);

#endif//__SimpleWork_NN_Operators_CSaveStateOperator_h__