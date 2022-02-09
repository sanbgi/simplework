#ifndef __SimpleWork_NN_Operators_CMinusOperator_h__
#define __SimpleWork_NN_Operators_CMinusOperator_h__

#include "operator.h"
static SCtx sCtx("MinusOperator");
class CMinusOperator : public CNnSolver {
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
            *pO = *pIn1 - *pIn2;
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
            *pDevia2 -= (*pDeviaO);
            pDevia1++, pDevia2++, pDeviaO++;
            if(pDevia1 == pDevia1End) {
                pDevia1 = (Q*)inVars[0].devia;
            }
            if(pDevia2 == pDevia2End) {
                pDevia2 = (Q*)inVars[1].devia;
            }
        }
    }

    int initSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
        if(idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return solveTwoEleWise(nInVars, pInVars, spVarOut);
    }
};

static SNnSolverRegister s_Register("minus", CNnSolver::createStaticSolver<CMinusOperator>);

#endif//__SimpleWork_NN_Operators_CMinusOperator_h__