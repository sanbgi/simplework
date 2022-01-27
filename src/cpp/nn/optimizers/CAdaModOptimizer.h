#ifndef __SimpleWork_NN_CAdaModOptimizer_H__
#define __SimpleWork_NN_CAdaModOptimizer_H__

#include "../nn.h"

//
// 参考：   https://zhuanlan.zhihu.com/p/102733819
//
template<typename Q>
class CAdaModOptimizer : protected CObject, IOptimizer {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOptimizer)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    void* getDeviationPtr(int nDeviations){
        if(nDeviations == 0) {
            return nullptr;
        }

        if(m_nDeviations != nDeviations) {
            m_spDeviations.take(new Q[nDeviations*4], [](Q* ptr) {
                delete[] ptr;
            });
            m_nDeviations = nDeviations;
            m_pDeviation = m_spDeviations;
            m_pMomentum = m_pDeviation + nDeviations;
            m_pVelocity = m_pMomentum + nDeviations;
            m_pS = m_pVelocity + nDeviations;
            memset(m_pDeviation, 0, sizeof(Q) * nDeviations * 4);
            resetVars();
        }
        return m_pDeviation;
    }

    int updateDeviation(int nBatchSize){

        const Q esp = 1e-8;
        const Q learnRate = 0.001;
        const Q beta1 = 0.9;
        const Q beta2 = 0.999;
        const Q beta3 = 0.99;

        //
        // 更新一阶、二阶动量校正参数
        //
        m_dBeta1Bais *= beta1;
        m_dBeta2Bais *= beta2;

        Q momentum, velocity, s;
        Q beta1Bais = m_dBeta1Bais;
        Q beta2Bais = m_dBeta2Bais;
        Q* pDeviation = m_pDeviation;
        Q* pMomentum = m_pMomentum;
        Q* pVelocity = m_pVelocity;
        Q* pS = m_pS;
        int nDeviation = m_nDeviations;
        for( int i=0; i<nDeviation; i++) {
            (*pDeviation) = (*pDeviation) / nBatchSize;
            (*pMomentum) = beta1 * (*pMomentum) + (1-beta1) * (*pDeviation);
            (*pVelocity) = beta2 * (*pVelocity) + (1-beta2) * (*pDeviation) * (*pDeviation);
            momentum = (*pMomentum) / ( 1 - beta1Bais);
            velocity = (*pVelocity) / ( 1 - beta2Bais);
            s = learnRate / (sqrt(velocity) + esp);
            (*pS) = beta3 * (*pS) + (1-beta3) * s;
            s = min(s, (*pS));
            (*pDeviation) = s * momentum;
            pMomentum++, pVelocity++, pDeviation++, pS++;
        }
        return sCtx.success();
    }

public:
    static int createOptimizer(SOptimizer& spOptimizer) {
        CPointer<CAdaModOptimizer> sp;
        CObject::createObject(sp);
        spOptimizer.setPtr(sp.getPtr());
        return sCtx.success();
    }

public:
    CAdaModOptimizer(){
        m_nDeviations = 0;
    }
    void resetVars() {
        m_dBeta1Bais = 1;
        m_dBeta2Bais = 1;
    }

private:
    int m_nDeviations;
    CTaker<Q*> m_spDeviations;
    Q* m_pDeviation;
    Q* m_pMomentum;
    Q* m_pVelocity;
    Q* m_pS;
    Q m_dBeta1Bais;
    Q m_dBeta2Bais;
};


#endif//__SimpleWork_NN_CAdaModOptimizer_H__