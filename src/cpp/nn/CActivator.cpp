#include "CActivator.h"
#include "math.h"
#include <map>
#include <string>

using namespace std;

map<string, CActivator*> s_mapActivators = {
    { "none", CActivator::getNoneActivator() },
    { "relu", CActivator::getReLU() },
    { "softmax", CActivator::getSoftmax() },
    { "sigmod", CActivator::getSigmod() },
    { "tanh", CActivator::getTanh() },
    { "elu", CActivator::getELU() },
    { "leakyrelu", CActivator::getLeakyReLU() },
};

CActivator* CActivator::getActivation(const char* szActivator) {
    if( szActivator != nullptr && *szActivator > 0) {
        map<string, CActivator*>::iterator it = s_mapActivators.find(szActivator);
        if( it != s_mapActivators.end() ) {
            return it->second;
        }
        return nullptr;
    }
    return getELU();
}


//
// 神经网络输出总是在[0,1]之间
//
CActivator* CActivator::getNoneActivator() {
    static class CActivatorImp : public CActivator {
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = pZArray[i];
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                pDzArray[i] = pYDeltaArray[i];
            }
        }
    }s_activator;
    return &s_activator;
}

//
// 神经网络输出总是在[0,1]之间
//
CActivator* CActivator::getNeuralActivator() {
    static class CActivatorImp : public CActivator {
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                double z = pZArray[i];
                pYArray[i] = z >= 0 ? ( z > 1 ? 1: z ) : 0;
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                if(pYArray[i]>0) {
                    if(pYArray[i] >= 1) 
                        pDzArray[i] = 0;//pYDeltaArray[i] >= 0 ? pYDeltaArray[i] : 0;
                    else    
                        pDzArray[i] = pYDeltaArray[i];
                }else{
                    pDzArray[i] = 0;
                }
                //pDzArray[i] = pYDeltaArray[i];
            }
        }
    }s_activator;
    return &s_activator;
}

CActivator* CActivator::getReLU() {
    static class CActivatorImp : public CActivator {
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = pZArray[i] >= 0 ? pZArray[i] : 0;
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                if(pYArray[i]>0) {
                    pDzArray[i] = pYDeltaArray[i];
                }else{
                    pDzArray[i] = 0;
                }
            }
        }
    }s_activator;
    return &s_activator;
}

static double s_leaky_a = 0.01;
CActivator* CActivator::getLeakyReLU() {
    static class CActivatorImp : public CActivator {
        static double activate(double x) {
            return x>=0?x:(s_leaky_a*x);
        }
        static double deactivate(double y) {
            //
            //  目标函数 = 求和(delta * delta) / 2，所以，输出相对于目标函数的偏导数刚
            //  好等于 delta，其中delta = Ycurrent - Yexpect 
            //
            return y>=0?1:s_leaky_a;
        }
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = activate(pZArray[i]);
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                pDzArray[i] = deactivate(pYArray[i]) * pYDeltaArray[i];
            }
        }
        double loss(int nData, double* pYArray, double* pYDeltaArray) {
            double dLoss = 0;
            for(int i=0; i<nData; i++) {
                dLoss += pYDeltaArray[i] * pYDeltaArray[i];
            }
            return dLoss/2.0;
        }
    }s_activator;
    return &s_activator;
}

CActivator* CActivator::getELU() {
    static class CActivatorImp : public CActivator {
        static double activate(double x) {
            return x>=0?x:(s_leaky_a*(exp(x)-1));
        }
        static double deactivate(double y) {
            //
            //  目标函数 = 求和(delta * delta) / 2，所以，输出相对于目标函数的偏导数刚
            //  好等于 delta，其中delta = Ycurrent - Yexpect 
            //
            return y>=0?1:(y+s_leaky_a);
        }
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = activate(pZArray[i]);
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                pDzArray[i] = deactivate(pYArray[i]) * pYDeltaArray[i];
            }
        }
        double loss(int nData, double* pYArray, double* pYDeltaArray) {
            double dLoss = 0;
            for(int i=0; i<nData; i++) {
                dLoss += pYDeltaArray[i] * pYDeltaArray[i];
            }
            return dLoss/2.0;
        }
    }s_activator;
    return &s_activator;
}

CActivator* CActivator::getSigmod() {
    static class CActivatorImp : public CActivator {
        static double activate(double x) {
            return 1/(1+exp(-x));
        }
        static double deactivate(double y) {
            return y*(1-y);
        }
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = activate(pZArray[i]);
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                pDzArray[i] = deactivate(pYArray[i]) * pYDeltaArray[i];
            }
        }
        double loss(int nData, double* pYArray, double* pYDeltaArray) {
            double dLoss = 0;
            for(int i=0; i<nData; i++) {
                dLoss += pYDeltaArray[i] * pYDeltaArray[i];
            }
            return dLoss/2.0;
        }
    }s_activator;
    return &s_activator;
}

CActivator* CActivator::getTanh() {
    static class CActivatorImp : public CActivator {
        static double activate(double x) {
            double v = exp(-2*x);
            return (1-v)/(1+v);
        }
        static double deactivate(double y) {
            return 1-y*y;
        }
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = activate(pZArray[i]);
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                pDzArray[i] = deactivate(pYArray[i]) * pYDeltaArray[i];
            }
        }
        double loss(int nData, double* pYArray, double* pYDeltaArray) {
            double dLoss = 0;
            for(int i=0; i<nData; i++) {
                dLoss += pYDeltaArray[i] * pYDeltaArray[i];
            }
            return dLoss/2.0;
        }
    }s_activator;
    return &s_activator;
}

CActivator* CActivator::getSoftmax() {
    static class CActivatorImp : public CActivator {
        void activate( int nData, double* pZArray, double* pYArray) {

            //
            // 参考：https://cloud.tencent.com/developer/article/1347841
            //
            
            //
            // 求最大值
            //
            double dMax = pZArray[0];
            for( int i=1; i<nData; i++ ){
                if(pZArray[i]>dMax) {
                    dMax = pZArray[i];
                }
            }

            //
            // 求各项和
            //
            double pExp[nData];
            double dSum = 0;
            for(int i=0; i<nData; i++) {
                pExp[i] = exp(pZArray[i]-dMax);
                dSum += pExp[i];
            }

            //
            // 归一化
            //
            for(int i=0; i<nData; i++) {
                pYArray[i] = pExp[i]/dSum;
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                //
                //  参考地址：https://blog.csdn.net/jiongjiongai/article/details/88324000
                //              https://blog.csdn.net/qq_42734797/article/details/110748836
                //
                pDzArray[i] = pYDeltaArray[i];
            }
        }
        double loss(int nData, double* pYArray, double* pYDeltaArray) {
            double dLoss = 0;
            for(int i=0; i<nData; i++) {
                dLoss -= (pYArray[i] + pYDeltaArray[i]) * log(pYArray[i]);
            }
            return dLoss;
        }
    }s_activator;
    return &s_activator;
}

