#ifndef __SimpleWork_NN_CAdagradOptimizer_H__
#define __SimpleWork_NN_CAdagradOptimizer_H__

#include "../nn.h"

//
// 参考：https://www.cnblogs.com/rinroll/p/12162342.html
//
template<typename Q=float> class CAdagradOptimizer : CObject, public IOptimizer {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOptimizer)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    void* getDeviationPtr(int nDeviations){
        if(nDeviations == 0) {
            return nullptr;
        }

        if(m_nDeviations != nDeviations) {
            m_spDeviations.take(new Q[nDeviations*2], [](Q* ptr) {
                delete[] ptr;
            });
            m_nDeviations = nDeviations;
            m_pDeviation = m_spDeviations;
            m_pVelocity = m_pDeviation + nDeviations;
            memset(m_pDeviation, 0, sizeof(Q) * nDeviations * 2);
        }
        return m_pDeviation;
    }

    int updateDeviation(int nBatchSize){
        const Q esp = 1e-8;
        const Q learnRate = 0.001;
        Q* pDeviation = m_pDeviation;
        Q* pVelocity = m_pVelocity;
        int nDeviation = m_nDeviations;
        for( int i=0; i<nDeviation; i++) {
            (*pDeviation) = (*pDeviation) / nBatchSize;
            (*pVelocity) = (*pVelocity) + (*pDeviation) * (*pDeviation);
            (*pDeviation) = learnRate * (*pDeviation) / (sqrt((*pVelocity)) + esp);
            pVelocity++, pDeviation++;
        }
        return sCtx.success();
    }

public:
    static int createOptimizer(SOptimizer& spOptimizer) {
        CPointer<CAdagradOptimizer<double>> sp;
        CObject::createObject(sp);
        spOptimizer.setPtr(sp.getPtr());
        return sCtx.success();
    }

public:
    CAdagradOptimizer(){
        m_nDeviations = 0;
    }

private:
    int m_nDeviations;
    CTaker<Q*> m_spDeviations;
    Q* m_pDeviation;
    Q* m_pVelocity;
};


#endif//__SimpleWork_NN_CAdagradOptimizer_H__