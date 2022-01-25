#ifndef __SimpleWork_NN_Operators_CStoreStateOperator_h__
#define __SimpleWork_NN_Operators_CStoreStateOperator_h__

#include "../CNnOperator.h"
#include "../CSize.h"

//
// 备份状态值到State中，并且在反向计算中，恢复偏导数
//
class CStoreStateOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn1End = pIn1 + inVars[0].size;
        Q* pIn2 = (Q*)inVars[1].data;
        while(pIn1 < pIn1End) {
            *pIn1 = *pIn2;
            pIn1++, pIn2++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
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

    int getEvalFunAddress(unsigned int idType, FEval& pEval, FEval& pDevia) {
        if(idType == CBasicData<float>::getStaticType() ) {
            pEval = evalT<float>;
            pDevia = deviaT<float>;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            pEval = evalT<double>;
            pDevia = deviaT<double>;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    static int createOperator(int nInVars, const SNnVariable pInVars[], SNnOperator& spOutVar) {
        if(nInVars != 2) {
            return sCtx.error("存储状态参数必须为两个，一个为状态变量，一个为计算变量");
        }

        SNnInternalVariable spState = pInVars[0];
        if( !spState || spState->getVariableType() != ENnVariableType::EVState ) {
            return sCtx.error("存储状态参数必须为两个，一个为状态变量，一个为计算变量");
        }

        CPointer<CStoreStateOperator> spOut;
        CObject::createObject(spOut);
        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }
};


#endif//__SimpleWork_NN_Operators_CStoreStateOperator_h__