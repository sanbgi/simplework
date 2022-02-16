#include "CActivator.h"
#include "math.h"
#include <map>
#include <string>

using namespace std;

template<typename Q> class CActivatorT {
public:
    //
    // 神经网络输出总是在[0,1]之间
    //
    static CActivator* getNoneActivator() {
        static class CActivatorImp : public CActivator {
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = ((Q*)pZArray)[i];
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pDzArray)[i] = ((Q*)pYDeltaArray)[i];
                }
            }
        }s_activator;
        return &s_activator;
    }

    //
    // 神经网络输出总是在[0,1]之间
    //
    static CActivator* getNeuralActivator() {
        static class CActivatorImp : public CActivator {
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    Q z = ((Q*)pZArray)[i];
                    ((Q*)pYArray)[i] = z >= 0 ? ( z > 1 ? 1: z ) : 0;
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    if(((Q*)pZArray)[i]>=0) {
                        if(((Q*)pZArray)[i] > 1) 
                            ((Q*)pDzArray)[i] = 0;
                        else    
                            ((Q*)pDzArray)[i] = ((Q*)pYDeltaArray)[i];
                    }else{
                        ((Q*)pDzArray)[i] = 0;
                    }
                }
            }
        }s_activator;
        return &s_activator;
    }

    static CActivator* getReLU() {
        static class CActivatorImp : public CActivator {
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = ((Q*)pZArray)[i] >= 0 ? ((Q*)pZArray)[i] : 0;
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    if(((Q*)pZArray)[i]>=0) {
                        ((Q*)pDzArray)[i] = ((Q*)pYDeltaArray)[i];
                    }else{
                        ((Q*)pDzArray)[i] = 0;
                    }
                }
            }
        }s_activator;
        return &s_activator;
    }

    static CActivator* getLeakyReLU() {
        static Q s_leaky_a = 0.01;
        static class CActivatorImp : public CActivator {
            static Q activate(Q x) {
                return x>=0?x:(s_leaky_a*x);
            }
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = activate(((Q*)pZArray)[i]);
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    Q dz = ((Q*)pZArray)[i] < 0 ? s_leaky_a : 1;
                    ((Q*)pDzArray)[i] = dz * ((Q*)pYDeltaArray)[i];
                }
            }
            Q loss(int nData, void* pYArray, void* pYDeltaArray) {
                Q dLoss = 0;
                for(int i=0; i<nData; i++) {
                    dLoss += ((Q*)pYDeltaArray)[i] * ((Q*)pYDeltaArray)[i];
                }
                return dLoss/2.0;
            }
        }s_activator;
        return &s_activator;
    }


    static CActivator* getELU() {
        static Q s_leaky_a = 0.01;
        static class CActivatorImp : public CActivator {
            static Q activate(Q x) {
                return x>=0?x:(s_leaky_a*(exp(x)-1));
            }
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = activate(((Q*)pZArray)[i]);
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    Q z = ((Q*)pZArray)[i];
                    Q dz = z >= 0 ? 1 : (activate(z) + s_leaky_a);
                    ((Q*)pDzArray)[i] = dz * ((Q*)pYDeltaArray)[i];
                }
            }
            Q loss(int nData, void* pYArray, void* pYDeltaArray) {
                Q dLoss = 0;
                for(int i=0; i<nData; i++) {
                    dLoss += ((Q*)pYDeltaArray)[i] * ((Q*)pYDeltaArray)[i];
                }
                return dLoss/2.0;
            }
        }s_activator;
        return &s_activator;
    }


    static CActivator* getSigmoid() {
        static class CActivatorImp : public CActivator {
            static Q activate(Q x) {
                return 1/(1+exp(-x));
            }
            static Q deactivate(Q y) {
                return y*(1-y);
            }
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = activate(((Q*)pZArray)[i]);
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    Q y = activate(((Q*)pZArray)[i]);
                    ((Q*)pDzArray)[i] = (y*(1-y)) * ((Q*)pYDeltaArray)[i];
                }
            }
            Q loss(int nData, void* pYArray, void* pYDeltaArray) {
                Q dLoss = 0;
                for(int i=0; i<nData; i++) {
                    dLoss += ((Q*)pYDeltaArray)[i] * ((Q*)pYDeltaArray)[i];
                }
                return dLoss/2.0;
            }
        }s_activator;
        return &s_activator;
    }


    static CActivator* getTanh() {
        static class CActivatorImp : public CActivator {
            static Q activate(Q x) {
                Q v = exp(-2*x);
                return (1-v)/(1+v);
            }
            void activate( int nData, void* pZArray, void* pYArray) {
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = activate(((Q*)pZArray)[i]);
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    Q y = activate(((Q*)pZArray)[i]);
                    ((Q*)pDzArray)[i] = (1-y*y) * ((Q*)pYDeltaArray)[i];
                }
            }
            Q loss(int nData, void* pYArray, void* pYDeltaArray) {
                Q dLoss = 0;
                for(int i=0; i<nData; i++) {
                    dLoss += ((Q*)pYDeltaArray)[i] * ((Q*)pYDeltaArray)[i];
                }
                return dLoss/2.0;
            }
        }s_activator;
        return &s_activator;
    }


    static CActivator* getSoftmax() {
        static class CActivatorImp : public CActivator {
            void activate( int nData, void* pZArray, void* pYArray) {

                //
                // 参考：https://cloud.tencent.com/developer/article/1347841
                //
                
                //
                // 求最大值
                //
                Q dMax = ((Q*)pZArray)[0];
                for( int i=1; i<nData; i++ ){
                    if(((Q*)pZArray)[i]>dMax) {
                        dMax = ((Q*)pZArray)[i];
                    }
                }

                //
                // 求各项和
                //
                Q pExp[nData];
                Q dSum = 0;
                for(int i=0; i<nData; i++) {
                    pExp[i] = exp(((Q*)pZArray)[i]-dMax);
                    dSum += pExp[i];
                }

                //
                // 归一化
                //
                for(int i=0; i<nData; i++) {
                    ((Q*)pYArray)[i] = pExp[i]/dSum;
                }
            }
            void deactivate( int nData, void* pZArray, void* pYDeltaArray, void* pDzArray) {
                for(int i=0; i<nData; i++) {
                    //
                    //  参考地址：https://blog.csdn.net/jiongjiongai/article/details/88324000
                    //              https://blog.csdn.net/qq_42734797/article/details/110748836
                    //
                    ((Q*)pDzArray)[i] = ((Q*)pYDeltaArray)[i];
                }
            }
            Q loss(int nData, void* pYArray, void* pYDeltaArray) {
                Q dLoss = 0;
                for(int i=0; i<nData; i++) {
                    dLoss -= (((Q*)pYArray)[i] + ((Q*)pYDeltaArray)[i]) * log(((Q*)pYArray)[i]);
                }
                return dLoss;
            }
        }s_activator;
        return &s_activator;
    }

};

map<string, CActivator*> s_mapDoubleActivators = {
    { "none", CActivatorT<double>::getNoneActivator() },
    { "relu", CActivatorT<double>::getReLU() },
    { "softmax", CActivatorT<double>::getSoftmax() },
    { "sigmoid", CActivatorT<double>::getSigmoid() },
    { "tanh", CActivatorT<double>::getTanh() },
    { "elu", CActivatorT<double>::getELU() },
    { "leakyrelu", CActivatorT<double>::getLeakyReLU() },
};

map<string, CActivator*> s_mapFloatActivators = {
    { "none", CActivatorT<float>::getNoneActivator() },
    { "relu", CActivatorT<float>::getReLU() },
    { "softmax", CActivatorT<float>::getSoftmax() },
    { "sigmoid", CActivatorT<float>::getSigmoid() },
    { "tanh", CActivatorT<float>::getTanh() },
    { "elu", CActivatorT<float>::getELU() },
    { "leakyrelu", CActivatorT<float>::getLeakyReLU() },
};

CActivator* CActivator::getActivation(unsigned int idType, const char* szActivator) {
    if( szActivator != nullptr && *szActivator > 0) {
        map<string, CActivator*>* pMap = nullptr;
        if(idType == CBasicData<double>::getStaticType()) {
            pMap = &s_mapDoubleActivators;
        }else
        if(idType == CBasicData<float>::getStaticType()) {
            pMap = &s_mapFloatActivators;
        }else{
            return nullptr;
        }
        map<string, CActivator*>::iterator it = pMap->find(szActivator);
        if( it != pMap->end() ) {
            return it->second;
        }
        return nullptr;
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return CActivatorT<double>::getNeuralActivator();
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return CActivatorT<float>::getNeuralActivator();
    }else{
        return nullptr;
    }
}
