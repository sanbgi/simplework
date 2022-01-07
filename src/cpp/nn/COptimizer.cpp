#include "COptimizer.h"
#include <map>
#include <string>
#include <math.h>

using namespace std;

SCtx sCtx("COptimizer");

//
// 参考：   https://zhuanlan.zhihu.com/p/102733819
//
class CAdaModOptimizer : CObject, IOptimizer {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOptimizer)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    double* getDeviationPtr(int nDeviations){
        if(nDeviations == 0) {
            return nullptr;
        }

        if(m_nDeviations != nDeviations) {
            m_spDeviations.take(new double[nDeviations*4], [](double* ptr) {
                delete[] ptr;
            });
            m_nDeviations = nDeviations;
            m_pDeviation = m_spDeviations;
            m_pMomentum = m_pDeviation + nDeviations;
            m_pVelocity = m_pMomentum + nDeviations;
            m_pS = m_pVelocity + nDeviations;
            memset(m_pDeviation, 0, sizeof(double) * nDeviations * 4);
            resetVars();
        }
        return m_pDeviation;
    }

    int updateDeviation(int nBatchSize){

        const double esp = 1e-8;
        const double learnRate = 0.001;
        const double beta1 = 0.9;
        const double beta2 = 0.999;
        const double beta3 = 0.99;

        //
        // 更新一阶、二阶动量校正参数
        //
        m_dBeta1Bais *= beta1;
        m_dBeta2Bais *= beta2;

        double momentum, velocity, s;
        double beta1Bais = m_dBeta1Bais;
        double beta2Bais = m_dBeta2Bais;
        double* pDeviation = m_pDeviation;
        double* pMomentum = m_pMomentum;
        double* pVelocity = m_pVelocity;
        double* pS = m_pS;
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
    CTaker<double*> m_spDeviations;
    double* m_pDeviation;
    double* m_pMomentum;
    double* m_pVelocity;
    double* m_pS;
    double m_dBeta1Bais;
    double m_dBeta2Bais;
};


//
// 参考：   https://zhuanlan.zhihu.com/p/61955391
//          https://www.cnblogs.com/rinroll/p/12162342.html
//
class CAdamOptimizer : CObject, IOptimizer {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOptimizer)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    double* getDeviationPtr(int nDeviations){
        if(nDeviations == 0) {
            return nullptr;
        }

        if(m_nDeviations != nDeviations) {
            m_spDeviations.take(new double[nDeviations*3], [](double* ptr) {
                delete[] ptr;
            });
            m_nDeviations = nDeviations;
            m_pDeviation = m_spDeviations;
            m_pMomentum = m_pDeviation + nDeviations;
            m_pVelocity = m_pMomentum + nDeviations;
            memset(m_pDeviation, 0, sizeof(double) * nDeviations * 3);
            resetVars();
        }
        return m_pDeviation;
    }

    int updateDeviation(int nBatchSize){

        const double esp = 1e-8;
        const double learnRate = 0.001;
        const double beta1 = 0.9;
        const double beta2 = 0.999;

        //
        // 更新一阶、二阶动量校正参数
        //
        m_dBeta1Bais *= beta1;
        m_dBeta2Bais *= beta2;

        double momentum, velocity;
        double beta1Bais = m_dBeta1Bais;
        double beta2Bais = m_dBeta2Bais;
        double* pDeviation = m_pDeviation;
        double* pMomentum = m_pMomentum;
        double* pVelocity = m_pVelocity;
        int nDeviation = m_nDeviations;
        for( int i=0; i<nDeviation; i++) {
            (*pDeviation) = (*pDeviation) / nBatchSize;
            (*pMomentum) = beta1 * (*pMomentum) + (1-beta1) * (*pDeviation);
            (*pVelocity) = beta2 * (*pVelocity) + (1-beta2) * (*pDeviation) * (*pDeviation);
            momentum = (*pMomentum) / ( 1 - beta1Bais);
            velocity = (*pVelocity) / ( 1 - beta2Bais);
            (*pDeviation) = learnRate * momentum / (sqrt(velocity) + esp);
            pMomentum++, pVelocity++, pDeviation++;
        }
        return sCtx.success();
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
    int m_nDeviations;
    CTaker<double*> m_spDeviations;
    double* m_pDeviation;
    double* m_pMomentum;
    double* m_pVelocity;
    double m_dBeta1Bais;
    double m_dBeta2Bais;
};

typedef int (*FCreateOptimizer)(SOptimizer& spOptimizer);
map<string, FCreateOptimizer> s_mapOptimizerFs = {
    { "adam", CAdamOptimizer::createOptimizer },
    { "adamop", CAdaModOptimizer::createOptimizer },
};

int COptimizer::getOptimizer(const char* szOptimizer, SOptimizer& spOptimizer) {
    if(szOptimizer) {
        map<string, FCreateOptimizer>::iterator it = s_mapOptimizerFs.find(szOptimizer);
        if( it != s_mapOptimizerFs.end() ) {
            return (*it->second)(spOptimizer);
        }
        return sCtx.error((std::string("不支持的优化器: ") + szOptimizer).c_str());
    }
    return CAdaModOptimizer::createOptimizer(spOptimizer);
}