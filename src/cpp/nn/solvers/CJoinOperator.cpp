#ifndef __SimpleWork_NN_Operators_CJoinOperator_h__
#define __SimpleWork_NN_Operators_CJoinOperator_h__

#include "operator.h"
static SCtx sCtx("JoinOperator");
class CJoinOperator : public CNnSolver {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size+inVars[1].size==outVar.size)

        Q* pIn1 = (Q*)inVars[0].data;
        int nIn1 = inVars[0].size;

        Q* pIn2 = (Q*)inVars[1].data;
        int nIn2 = inVars[1].size;

        int iIn1, iIn2;
        Q* pO = (Q*)outVar.data;
        while(nBatchs-->0) {
            iIn1 = nIn1;
            while(iIn1-->0) {
                *pO = *pIn1;
                pO++, pIn1++;
            }

            iIn2 = nIn2;
            while(iIn2-->0) {
                *pO = *pIn2;
                pO++, pIn2++;
            }
        }
        VERIFY(pO-(Q*)outVar.data==outVar.size)
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size+inVars[1].size==outVar.size)

        Q* pDevia1 = (Q*)inVars[0].devia;
        int nIn1 = inVars[0].size;

        Q* pDevia2 = (Q*)inVars[1].devia;
        int nIn2 = inVars[1].size;

        int iIn1, iIn2;
        Q* pDeviaO = (Q*)outVar.devia;
        while(nBatchs-->0) {
            iIn1 = nIn1;
            while(iIn1-->0) {
                *pDevia1 += *pDeviaO;
                pDevia1++, pDeviaO++;
            }

            iIn2 = nIn2;
            while(iIn2-->0) {
                *pDevia2 += *pDeviaO;
                pDevia2++, pDeviaO++;
            }
        }
        VERIFY(pDeviaO-(Q*)outVar.devia==outVar.size)
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
        if(nInVars != 2) {
            return sCtx.error("连接需要两个参数");
        }

        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        if( spDim1.size() != 1 || spDim2.size() != 1) {
            return sCtx.error("连接只适用于连接两个向量");
        }

        int size1 = *spDim1.data();
        int size2 = *spDim2.data();
        int size = size1 + size2;
        return createVariable(SDimension(1,&size), spVarOut);
    }
};

static SNnSolverRegister s_Register("join", CNnSolver::createStaticSolver<CJoinOperator>);

#endif//__SimpleWork_NN_Operators_CJoinOperator_h__