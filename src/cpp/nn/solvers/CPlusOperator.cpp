#ifndef __SimpleWork_NN_Operators_CPlusOperator_h__
#define __SimpleWork_NN_Operators_CPlusOperator_h__


#include "operator.h"

static SCtx sCtx("PlusOperator");
class CPlusOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:

    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        Q* pIn1End = pIn1+inVars[0].size;
        Q* pIn2End = pIn2+inVars[1].size;
        while(outVar.size-->0) {
            *pO = *pIn1 + *pIn2;
            pO++, pIn1++, pIn2++;
            if(pIn1 == pIn1End) {
                pIn1 = (Q*)inVars[0].data;
            }
            if(pIn2 == pIn2End) {
                pIn2 = (Q*)inVars[1].data;
            }
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDeviaO = (Q*)outVar.devia;
        Q* pDevia1End = pDevia1 + inVars[0].size;
        Q* pDevia2End = pDevia2 + inVars[1].size;
        while(outVar.size-->0) {
            *pDevia1 += (*pDeviaO);
            *pDevia2 += (*pDeviaO);
            pDevia1++, pDevia2++, pDeviaO++;
            if(pDevia1 == pDevia1End) {
                pDevia1 = (Q*)inVars[0].devia;
            }
            if(pDevia2 == pDevia2End) {
                pDevia2 = (Q*)inVars[1].devia;
            }
        }
    }

    template<typename Q>
    static void evalT_OpenCL(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        Q* pIn1End = pIn1+inVars[0].size;
        Q* pIn2End = pIn2+inVars[1].size;
        while(outVar.size-->0) {
            *pO = *pIn1 + *pIn2;
            pO++, pIn1++, pIn2++;
            if(pIn1 == pIn1End) {
                pIn1 = (Q*)inVars[0].data;
            }
            if(pIn2 == pIn2End) {
                pIn2 = (Q*)inVars[1].data;
            }
        }
    }

    int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) {
        solveParameter.eClRange = PSolveFunc::POut;
        if(solveCtx.idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.nParamterSize = 0;
            solveParameter.pParameterData = this;
            return sCtx.success();
        }else if(solveCtx.idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.nParamterSize = 0;
            solveParameter.pParameterData = this;
            return sCtx.success();
        }
        return sCtx.error("????????????");
    }    
    
    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return solveTwoEleWise(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getName() { return "Plus"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.PlusSolver"; }

};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CPlusOperator, CPlusOperator::__getClassKey())
static SNnSolverRegister s_Register("plus", CNnSolver::createStaticSolver<CPlusOperator>);

#endif//__SimpleWork_NN_Operators_CPlusOperator_h__