
#include <math.h>
#include <vector>
#include <iostream>
#include "CNeuralNetwork.h"

using namespace sw;
using namespace sw::av;
using namespace sw::math;
using namespace sw::nn;

static SCtx sCtx("CNeuralNetwork.Test");
void CNeuralNetwork::run() {

    SNeuralPipe spImageReader = SNeuralNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\train-images.gz");
    SNeuralPipe spLabelReader = SNeuralNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\train-labels.gz");

    //
    // 一次读取10个
    //
    STensor spPipeIn = STensor::createValue(10);
    SNeuralNetwork nn = createNetwork();
    //SNeuralNetwork nn = createRotNetwork();

    double sumAcc = 0;
    double sumLoss = 0;
    double sumX = 0.98;
    int nHit = 0;

    STensor spBatchImage, spBatchLabel;
    while( spImageReader->push(spPipeIn, spBatchImage) == sCtx.success() && spLabelReader->push(spPipeIn, spBatchLabel) == sCtx.success() ) {
        // 
        // 分类信息
        //
        STensor spClassify = SNeuralNetwork::classifyTensor(10, spBatchLabel);

        //
        // 图片信息，将字节类型图片张量，转化为[0,1)浮点类型张量
        //
        STensor spIn = SNeuralNetwork::normalizeTensor(spBatchImage);

        //
        // 神经网络求解
        //
        STensor spOut = nn.eval(spIn);

        //
        // 计算偏差量
        //
        STensor spOutDeviation = spOut - spClassify;

        //
        // 学习更新神经网络
        //
        nn.learn(spOut, spOutDeviation);

        //
        // 打印一些结果信息
        //
        {
            int nOutDeviation = spOutDeviation->getDataSize();
            double* pOutDeviation = spOutDeviation->getDataPtr<double>();
            double* pOutTarget = spClassify->getDataPtr<double>();
            int nAcc = 0;
            double xAcc = 0;
            double delta = 0;
            for(int i=0; i<nOutDeviation; i++) {
                if( pOutTarget[i] > 0.9 ) {
                    if(pOutDeviation[i] > -0.1) {
                        nAcc++;
                        nHit++;
                    }
                    xAcc += abs(pOutDeviation[i]) / nOutDeviation * 10;
                }
                delta += abs(pOutDeviation[i]) / nOutDeviation;
            }
            sumAcc = sumAcc * sumX + (1-xAcc) * (1-sumX);
            sumLoss = sumLoss * sumX + delta * (1-sumX);
            static int t = 0;
            if( t++ % 10 == 0) {
                std::cout   << "\rt:" << t << ",\tloss:" << delta <<",\tsloss:"<< sumLoss 
                            <<",\tnAcc:" << nAcc << ", \tavgAccDelta:" << xAcc<< "\tsAcc:"
                            << sumAcc << ",\tavgAcc:" << nHit / 10.0 / t  << "\n";
            }
        }
    }
}

SNeuralNetwork CNeuralNetwork::createNetwork() {
    std::vector<SNeuralNetwork> arrParallelNets;
    arrParallelNets.push_back(SNeuralNetwork::createConv(5,5,16));
    arrParallelNets.push_back(SNeuralNetwork::createConv(5,5,16));

    std::vector<SNeuralNetwork> arrNets;
    //arrNets.push_back(SNeuralNetwork::createConv(5,5,32));
    arrNets.push_back(SNeuralNetwork::createParallel(arrParallelNets.size(),arrParallelNets.data()));
    arrNets.push_back(SNeuralNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNeuralNetwork::createConv(7,7,64));
    arrNets.push_back(SNeuralNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNeuralNetwork::createDense(576));
    //arrNets.push_back(SNeuralNetwork::createDense(10));
    arrNets.push_back(SNeuralNetwork::createDense(10, "softmax"));
    return SNeuralNetwork::createSequence(arrNets.size(), arrNets.data());
}

SNeuralNetwork CNeuralNetwork::createRotNetwork() {
    std::vector<SNeuralNetwork> arrNets;
    arrNets.push_back(SNeuralNetwork::createRotConv(7,7,8,0,0));
    arrNets.push_back(SNeuralNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNeuralNetwork::createRotConv(7,7,32,0,0));
    arrNets.push_back(SNeuralNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNeuralNetwork::createDense(576));
    //arrNets.push_back(SNeuralNetwork::createDense(10));
    arrNets.push_back(SNeuralNetwork::createDense(10, "softmax"));
    return SNeuralNetwork::createSequence(arrNets.size(), arrNets.data());
}
