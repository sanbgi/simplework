#ifndef __SimpleWork_NN_CAdamOptimizer_H__
#define __SimpleWork_NN_CAdamOptimizer_H__

#include "../nn.h"

//
// 参考：   https://zhuanlan.zhihu.com/p/61955391
//          https://www.cnblogs.com/rinroll/p/12162342.html
//
template<typename Q>
class CAdamOptimizer : protected CObject, IOptimizer {

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
            m_pS = m_pMomentum + nDeviations;
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
        Q* pVelocity = m_pS;
        int nDeviation = m_nDeviations;
        for( int i=0; i<nDeviation; i++) {
            (*pDeviation) = (*pDeviation) / nBatchSize;
            (*pMomentum) = beta1 * (*pMomentum) + (1-beta1) * (*pDeviation);
            (*pVelocity) = beta2 * (*pVelocity) + (1-beta2) * (*pDeviation) * (*pDeviation);
            momentum = (*pMomentum) / ( 1 - beta1Bais);
            velocity = (*pVelocity) / ( 1 - beta2Bais);
            (*pDeviation) = learnRate * momentum / ((Q)sqrt(velocity) + esp);
            pMomentum++, pVelocity++, pDeviation++;
        }
        return sCtx.success();
    }

    int updateDeviation(PDATATYPE idType, int nBatchs, const SDevice& spDevice, int nDeviations, void* pDeviations ) {
        if(nDeviations != m_nDeviations) {
            int nBytes = nDeviations*sizeof(Q);
            m_spMomentum = SDeviceMemory::createDeviceMemory(spDevice, nBytes, nullptr);
            m_spVelocity = SDeviceMemory::createDeviceMemory(spDevice, nBytes, nullptr);
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
        PKernelVariable pArgs[] = {
            nBatchs,
            m_dBeta1Bais,
            m_dBeta2Bais,
            pDeviations,
            m_spMomentum.data(spDevice),
            m_spVelocity.data(spDevice),
        };
        static int sKernelId = 0;
        switch(CBasicData<Q>::getStaticType()) {
            case PDATATYPE_FLOAT:{
                static PRuntimeKey sKernelKey("sw.nn.Adam.floatEval");
                return spDevice->runKernel( sKernelKey, 6, pArgs, 1, &nDeviations);
                }
            case PDATATYPE_DOUBLE:{
                static PRuntimeKey sKernelKey("sw.nn.Adam.doubleEval");
                return spDevice->runKernel( sKernelKey, 6, pArgs, 1, &nDeviations);
                }
        }
        return sCtx.error("数据类型暂时不支持");
    }

public:
    static int createOptimizer(SOptimizer& spOptimizer) {
        CPointer<CAdamOptimizer> sp;
        CObject::createObject(sp);
        spOptimizer.setPtr(sp.getPtr());
        return sCtx.success();
    }

public:
    CAdamOptimizer(){
        m_nDeviations = 0;
    }
    void resetVars() {
        m_dBeta1Bais = 1;
        m_dBeta2Bais = 1;
    }

private:
    CTaker<Q*> m_spDeviations;
    Q* m_pDeviation;
    Q* m_pMomentum;
    Q* m_pS;

    int m_nDeviations;
    Q m_dBeta1Bais;
    Q m_dBeta2Bais;

    SDeviceMemory m_spMomentum;
    SDeviceMemory m_spVelocity;
};


#endif//__SimpleWork_NN_CAdamOptimizer_H__