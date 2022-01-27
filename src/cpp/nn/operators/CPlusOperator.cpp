#ifndef __SimpleWork_NN_Operators_CPlusOperator_h__
#define __SimpleWork_NN_Operators_CPlusOperator_h__

#include "operator.h"

static SCtx sCtx("PlusOperator");
class CPlusOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size % inVars[1].size == 0 && inVars[0].size==outVar.size)
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        Q* pOEnd = pO + outVar.size;
        Q* pItIn2;
        Q* pItIn2End = pIn2 + inVars[1].size;
        while(pO < pOEnd) {
            pItIn2 = pIn2;
            while(pItIn2 < pItIn2End) {
                *pO = *pIn1 + *pItIn2;
                pO++, pIn1++, pItIn2++;
            }
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size % inVars[1].size == 0 && inVars[0].size==outVar.size)
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDeviaO = (Q*)outVar.devia;
        Q* pDeviaOEnd = pDeviaO + outVar.size;
        Q* pItDevia2, *pItDevia2End = pDevia2 + inVars[1].size;
        while(pDeviaO < pDeviaOEnd) {
            pItDevia2 = pDevia2;
            while(pItDevia2 < pItDevia2End) {
                *pDevia1 += *pDeviaO;
                *pItDevia2 += *pDeviaO;
                pDevia1++, pItDevia2++, pDeviaO++;
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

static SNnOperatorRegister s_Register("plus", CNnOperator::createStaticOperator<CPlusOperator>);

#endif//__SimpleWork_NN_Operators_CPlusOperator_h__