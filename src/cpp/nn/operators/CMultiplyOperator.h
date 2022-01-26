#ifndef __SimpleWork_NN_Operators_CMultiplyOperator_h__
#define __SimpleWork_NN_Operators_CMultiplyOperator_h__

#include "../CNnOperator.h"

class CMultiplyOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        Q* pOEnd = pO + outVar.size;
        while(pO < pOEnd) {
            *pO = (*pIn1) * (*pIn2);
            pO++, pIn1++, pIn2++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDeviaO = (Q*)outVar.devia;
        Q* pDeviaOEnd = pDeviaO + outVar.size;
        while(pDeviaO < pDeviaOEnd) {
            *pDevia1 += (*pDeviaO)*(*pIn2);
            *pDevia2 += (*pDeviaO)*(*pIn1);
            pDevia1++, pDevia2++, pDeviaO++;
            pIn1++, pIn2++;
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

    int solve(int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return solveTwoEleWise(nInVars, pInVars, spVarOut);
    }
};

#endif//__SimpleWork_NN_Operators_CMultiplyOperator_h__