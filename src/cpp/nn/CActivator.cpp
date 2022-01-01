#include "CActivator.h"
#include "math.h"

CActivator* CActivator::getActivation(SNeuralNetwork::EACTIVATION eActivation) {
    switch(eActivation) {
        case SNeuralNetwork::ACTIVATION_ReLU:
            return getReLU();

        case SNeuralNetwork::ACTIVATION_Softmax:
            return getSoftmax();
    }
    return getReLU();
}

CActivator* CActivator::getReLU() {
    static class CActivatorImp : public CActivator {

        static double activate(double x) {
            return x>=0?x:0;
        }
        static double deactivate(double y) {
            //
            //  目标函数 = 求和(delta * delta) / 2，所以，输出相对于目标函数的偏导数刚
            //  好等于 -delta，其中delta = target - current 
            //
            return y>0?1:0;
        }
        void activate( int nData, double* pZArray, double* pYArray) {
            for(int i=0; i<nData; i++) {
                pYArray[i] = activate(pZArray[i]);
            }
        }
        void deactivate( int nData, double* pYArray, double* pYDeltaArray, double* pDzArray) {
            for(int i=0; i<nData; i++) {
                pDzArray[i] = deactivate(pYArray[i]) * (-pYDeltaArray[i]);
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

static double s_leaky_a = 0.01;
CActivator* CActivator::getLeakyReLU() {
    static class CActivatorImp : public CActivator {
        static double activate(double x) {
            return x>=0?x:(s_leaky_a*x);
        }
        static double deactivate(double y) {
            //
            //  目标函数 = 求和(delta * delta) / 2，所以，输出相对于目标函数的偏导数刚
            //  好等于 -delta，其中delta = target - current 
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
                pDzArray[i] = deactivate(pYArray[i]) * (-pYDeltaArray[i]);
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
            //  好等于 -delta，其中delta = target - current 
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
                pDzArray[i] = deactivate(pYArray[i]) * (-pYDeltaArray[i]);
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
                pDzArray[i] = deactivate(pYArray[i]) * (-pYDeltaArray[i]);
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
            return 2/(1+exp(-2*x)-1);
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
                pDzArray[i] = deactivate(pYArray[i]) * (-pYDeltaArray[i]);
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
                //  ?, 参考地址：https://blog.csdn.net/jiongjiongai/article/details/88324000
                //              https://blog.csdn.net/qq_42734797/article/details/110748836
                //  究竟应该是正，还是负，两篇文章结果不一样，选择相信后者
                //
                pDzArray[i] = -pYDeltaArray[i];
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
