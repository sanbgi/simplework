#ifndef __SimpleWork_NN_Operators_CMultiplyOperator_h__
#define __SimpleWork_NN_Operators_CMultiplyOperator_h__

#include "operator.h"
static SCtx sCtx("MultiplyOperator");
class CMultiplyOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size % inVars[1].size == 0 && inVars[0].size==outVar.size)
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        int iBatchSize, nBatchSize = outVar.size / nBatchs;
        int nMovebackIn1 = inVars[0].size == nBatchSize ? nBatchSize : 0;
        int nMovebackIn2 = inVars[1].size == nBatchSize ? nBatchSize : 0;
        while(nBatchs-->0) {
            iBatchSize = 0;
            while(iBatchSize++ < nBatchSize) {
                *pO = *pIn1 * *pIn2;
                pO++, pIn1++, pIn2++;
            }
            pIn1 -= nMovebackIn1;
            pIn2 -= nMovebackIn2;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size % inVars[1].size == 0 && inVars[0].size==outVar.size)
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDeviaO = (Q*)outVar.devia;
        int iBatchSize, nBatchSize = outVar.size / nBatchs;
        int nMovebackIn1 = inVars[0].size == nBatchSize ? nBatchSize : 0;
        int nMovebackIn2 = inVars[1].size == nBatchSize ? nBatchSize : 0;
        while(nBatchs-->0) {
            iBatchSize = 0;
            while(iBatchSize++ < nBatchSize) {
                *pDevia1 += (*pDeviaO) * (*pIn2);
                *pDevia2 += (*pDeviaO) * (*pIn1);
                pDevia1++, pDevia2++, pDeviaO++;
                pIn1++, pIn2++;
            }
            pIn1 -= nMovebackIn1;
            pDevia1 -= nMovebackIn1;
            pIn2 -= nMovebackIn2;
            pDevia2 -= nMovebackIn2;
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