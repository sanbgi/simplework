#ifndef __SimpleWork_NN_Operators_CLoadStateOperator_h__
#define __SimpleWork_NN_Operators_CLoadStateOperator_h__

#include "operator.h"
static SCtx sCtx("LoadStateOperator");

//
// 将状态值拷贝到变量中
//  inVar[0] : 普通变量
//  inVar[1] : 状态变量
//
class CLoadStateOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size == outVar.size)
        Q* pIn = (Q*)inVars[0].data;
        Q* pInEnd = pIn + inVars[0].size;
        Q* pOut = (Q*)outVar.data;
        while(pIn < pInEnd) {
            *pOut = *pIn;
            pIn++, pOut++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size == outVar.size)
        Q* pInDevia = (Q*)inVars[0].devia;
        Q* pInDeviaEnd = pInDevia + inVars[0].size;
        Q* pOutDevia = (Q*)outVar.devia;
        while(pInDevia < pInDeviaEnd) {
            *pInDevia += *pOutDevia;
            pInDevia++, pOutDevia++;
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
        if(nInVars != 1) {
            return sCtx.error("存储状态参数必须为两个，一个为状态变量，一个为计算变量");
        }
        return createVariable(pInVars[0].dimension(), spVarOut);
    }
};

static SNnOperatorRegister s_Register("loadState", CNnOperator::createStaticOperator<CLoadStateOperator>);

#endif//__SimpleWork_NN_Operators_CLoadStateOperator_h__