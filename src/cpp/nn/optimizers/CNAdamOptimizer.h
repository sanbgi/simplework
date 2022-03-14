#ifndef __SimpleWork_NN_CNAdamOptimizer_H__
#define __SimpleWork_NN_CNAdamOptimizer_H__

#include "../nn.h"

//
// 参考：https://cloud.tencent.com/developer/article/1468547
//
template<typename Q>
class CNAdamOptimizer : protected CObject, IOptimizer {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOptimizer)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    void* getDeviationPtr(int nDeviations){
        if(nDeviations == 0) {
            return nullptr;
        }

        if(m_nDeviations != nDeviations) {
            m_spDeviations.take(new Q[nDeviations*3], [](Q* ptr) {
                delete[] ptr;
            });
            m_nDeviations = nDeviations;
            m_pDeviation = m_spDeviations;
            m_pMomentum = m_pDeviation + nDeviations;
            m_pVelocity = m_pMomentum + nDeviations;
            memset(m_pDeviation, 0, sizeof(Q) * nDeviations * 3);
            resetVars();
        }
        return m_pDeviation;
    }

    int updateDeviation(int nBatchSize){

        const Q esp = 1e-8f;
        const Q learnRate = 0.001f;
        const Q beta1 = 0.9f;
        const Q beta2 = 0.999f;

        //
        // 更新一阶、二阶动量校正参数
        //
        m_dBeta1Bais *= beta1;
        m_dBeta2Bais *= beta2;

        Q momentum, velocity;
        Q beta1Bais = m_dBeta1Bais;
        Q beta2Bais = m_dBeta2Bais;
        Q* pDeviation = m_pDeviation;
        Q* pMomentum = m_pMomentum;
        Q* pVelocity = m_pVelocity;
        int nDeviation = m_nDeviations;
        for( int i=0; i<nDeviation; i++) {
            (*pDeviation) = (*pDeviation) / nBatchSize;
            (*pMomentum) = beta1 * (*pMomentum) + (1-beta1) * (*pDeviation);
            (*pVelocity) = beta2 * (*pVelocity) + (1-beta2) * (*pDeviation) * (*pDeviation);
            momentum = (*pMomentum) / ( 1 - beta1Bais);
            velocity = (*pVelocity) / ( 1 - beta2Bais);
            //Adam : (*pDeviation) = learnRate / (sqrt(velocity) + esp) * momentum;
            (*pDeviation) = learnRate / ((Q)sqrt(velocity) + esp) * ( beta1 * momentum + (1-beta1) * (*pDeviation) / (1-beta1Bais));
            pMomentum++, pVelocity++, pDeviation++;
        }
        return sCtx.success();
    }

    int updateDeviation(PDATATYPE idType, int nBatchs, const SDevice& spDevice, int nDeviations, void* pDeviations ) {
        if(nDeviations != m_nDeviations) {
            int nBytes = nDeviations*sizeof(Q);
            m_spMomentum = SDeviceMemory::createDeviceMemory(spDevice,nBytes, nullptr);
            m_spVelocity = SDeviceMemory::createDeviceMemory(spDevice,nBytes, nullptr);
            if(!m_spMomentum || !m_spVelocity) {
                m_nDeviations = 0;
                return sCtx.error("创建内存异常");
            }

            if( spDevice.memoryZero(m_spMomentum.data(spDevice), 0, nBytes) != sCtx.success() ||
                spDevice.memoryZero(m_spVelocity.data(spDevice), 0, nBytes) != sCtx.success() ) {
                return sCtx.error("初始化动量异常");
            }
            
            m_nDeviations = nDeviations;
            m_dBeta1Bais = 1;
            m_dBeta2Bais = 1;
        }

        Q esp = 1e-8f;
        Q learnRate = 0.001f;
        Q beta1 = 0.9f;
        Q beta2 = 0.999f;

        m_dBeta1Bais *= beta1;
        m_dBeta2Bais *= beta2;
        PKernalVariable pArgs[] = {
            nBatchs,
            m_dBeta1Bais,
            m_dBeta2Bais,
            pDeviations,
            m_spMomentum.data(spDevice),
            m_spVelocity.data(spDevice),
        };
        static int sKernalId = 0;
        switch(CBasicData<Q>::getStaticType()) {
            case PDATATYPE_FLOAT:
                return spDevice->runKernel( {&sKernalId, "sw.nn.NAdam.floatEval"}, 6, pArgs, 1, &nDeviations);
            case PDATATYPE_DOUBLE:
                return spDevice->runKernel( {&sKernalId, "sw.nn.NAdam.doubleEval"}, 6, pArgs, 1, &nDeviations);
        }
        return sCtx.error("数据类型暂时不支持");
    }

public:
    static int createOptimizer(SOptimizer& spOptimizer) {
        CPointer<CNAdamOptimizer> sp;
        CObject::createObject(sp);
        spOptimizer.setPtr(sp.getPtr());
        return sCtx.success();
    }

public:
    CNAdamOptimizer(){
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
    Q m_dBeta1Bais;
    Q m_dBeta2Bais;

    SDeviceMemory m_spMomentum;
    SDeviceMemory m_spVelocity;
};


#endif//__SimpleWork_NN_CNAdamOptimizer_H__