#ifndef __SimpleWork_NN_Operators_CMultiplyOperator_h__
#define __SimpleWork_NN_Operators_CMultiplyOperator_h__

#include "operator.h"
static SCtx sCtx("MultiplyOperator");
class CMultiplyOperator : public CNnOperator {
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
            *pO = *pIn1 * *pIn2;
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
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDeviaO = (Q*)outVar.devia;
        Q* pDevia1End = pDevia1 + inVars[0].size;
        Q* pDevia2End = pDevia2 + inVars[1].size;
        while(outVar.size-->0) {
            *pDevia1 += (*pDeviaO) * (*pIn2);
            *pDevia2 += (*pDeviaO) * (*pIn1);
            pDevia1++, pDevia2++, pDeviaO++;
            pIn1++, pIn2++;
            if(pDevia1 == pDevia1End) {
                pIn1 = (Q*)inVars[0].data;
                pDevia1 = (Q*)inVars[0].devia;
            }
            if(pDevia2 == pDevia2End) {
                pIn2 = (Q*)inVars[1].data;
                pDevia2 = (Q*)inVars[1].devia;
            }
        }
    }

    int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
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

static SNnOperatorRegister s_Register("multiply", CNnOperator::createStaticOperator<CMultiplyOperator>);

#endif//__SimpleWork_NN_Operators_CMultiplyOperator_h__