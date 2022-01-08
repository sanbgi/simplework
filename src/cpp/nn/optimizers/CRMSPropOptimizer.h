#ifndef __SimpleWork_NN_CRMSPropOptimizer_H__
#define __SimpleWork_NN_CRMSPropOptimizer_H__

#include "../nn.h"

//
// 参考：https://www.cnblogs.com/rinroll/p/12162342.html
//
class CRMSPropOptimizer : CObject, IOptimizer {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOptimizer)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    double* getDeviationPtr(int nDeviations){
        if(nDeviations == 0) {
            return nullptr;
        }

        if(m_nDeviations != nDeviations) {
            m_spDeviations.take(new double[nDeviations*2], [](double* ptr) {
                delete[] ptr;
            });
            m_nDeviations = nDeviations;
            m_pDeviation = m_spDeviations;
            m_pVelocity = m_pDeviation + nDeviations;
            memset(m_pDeviation, 0, sizeof(double) * nDeviations * 2);
        }
        return m_pDeviation;
    }

    int updateDeviation(int nBatchSize){
        const double esp = 1e-6;
        const double learnRate = 0.001;
        const double decayRate = 0.99;
        double* pDeviation = m_pDeviation;
        double* pVelocity = m_pVelocity;
        int nDeviation = m_nDeviations;
        for( int i=0; i<nDeviation; i++) {
            (*pDeviation) = (*pDeviation) / nBatchSize;
            (*pVelocity) = decayRate * (*pVelocity) + (1-decayRate) * (*pDeviation) * (*pDeviation);
            (*pDeviation) = learnRate * (*pDeviation) / (sqrt((*pVelocity)) + esp);
            pVelocity++, pDeviation++;
        }
        return sCtx.success();
    }

public:
    static int createOptimizer(SOptimizer& spOptimizer) {
        CPointer<CRMSPropOptimizer> sp;
        CObject::createObject(sp);
        spOptimizer.setPtr(sp.getPtr());
        return sCtx.success();
    }

public:
    CRMSPropOptimizer(){
        m_nDeviations = 0;
    }

private:
    int m_nDeviations;
    CTaker<double*> m_spDeviations;
    double* m_pDeviation;
    double* m_pVelocity;
};


#endif//__SimpleWork_NN_CRMSPropOptimizer_H__