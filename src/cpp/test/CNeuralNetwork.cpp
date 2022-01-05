
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

    STensor spPipeIn = STensor::createValue(10);
    SNeuralNetwork nn = createNetwork();

    STensor spBatchImage;
    STensor spBatchLabel;
    while( spImageReader->push(spPipeIn, spBatchImage) == sCtx.Success() && spLabelReader->push(spPipeIn, spBatchLabel) == sCtx.Success() ) {
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
                if( pOutTarget[i] > 0.9999999 ) {
                    if(pOutDeviation[i] > -0.1) {
                        nAcc++;
                    }
                    xAcc += -pOutDeviation[i]/10;
                }
                delta += pOutDeviation[i] * pOutDeviation[i];
            }
            static int t = 0;
            if( t++ % 10 == 0) {
                std::cout << "\rtrain:" << t << ", delta :" << delta <<", nAcc:" << nAcc << ", xAcc:" << xAcc<< "\n";
            }
        }
    }
}

SNeuralNetwork CNeuralNetwork::createNetwork() {
    std::vector<SNeuralNetwork> arrNets;
    arrNets.push_back(SNeuralNetwork::createConv(5,5,32));
    arrNets.push_back(SNeuralNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNeuralNetwork::createConv(7,7,64));
    arrNets.push_back(SNeuralNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNeuralNetwork::createDense(576));
    //arrNets.push_back(SNeuralNetwork::createDense(10));
    arrNets.push_back(SNeuralNetwork::createDense(10, SNeuralNetwork::ACTIVATION_Softmax));
    return SNeuralNetwork::createSequence(arrNets.size(), arrNets.data());
}

void CNeuralNetwork::runConv() {

    SNeuralNetwork spNet = SNeuralNetwork::createConv(2,2,1);
    for(int i=0; i<100; i++) {
        PTensor inputTensor;
        inputTensor.idType = CBasicData<double>::getStaticType();
        
        int pDimSize[4] = { 2, 3, 3, 2 };
        double pData[36] = { 
            1, 1,  0, 1, 0, 1,
            0, 1,  1, 0, 1, -1,
            1, 0, -1, 1, -1, 0,

            0, 1, 1, 1, 0, 1,
            1, 1, 0, 0, 1, 2,
            2, 1, 0, 0, 2, 0
        };
        inputTensor.nDims = 4;
        inputTensor.pDimSizes = pDimSize;
        inputTensor.nData = 36;
        inputTensor.pData = pData;
        struct CCtx : IVisitor<const PTensor&> {
            int visit(const PTensor& t) {
                return SError::ERRORTYPE_SUCCESS;
            }
        }ctx;
        //spNet->eval(inputTensor, &ctx);
    }
}


void CNeuralNetwork::runPool() {

    SNeuralNetwork spNet = SNeuralNetwork::createPool(2,2,2,2);
    for(int i=0; i<100; i++) {
        PTensor inputTensor;
        inputTensor.idType = CBasicData<double>::getStaticType();
        
        int pDimSize[4] = { 2, 4, 4, 2 };
        double pData[64] = { 
            1.1, 1.1,  1.2, 1.11, 2.3,  1.2, 2.2,  1.0,
            1.0, 1.4,  0.8, 1.30, 1.1, -1.0, 1, -3.0,
            0, 1,  1, 1, 0,  1, 1,  1,
            0, 1,  2, 2, 1,  0, 0,  2,
            1, 1,  0, 1, 0,  1, 0,  1,
            0, 1, -1, 1, 0, -1, 1, -1,
            1.1, 1.1,  1.2, 1.11, 2.3,  1.2, 2.2,  1.0,
            1.0, 1.4,  0.8, 1.30, 1.1, -1.0, 1, -3.0,
        };
        inputTensor.nDims = 4;
        inputTensor.pDimSizes = pDimSize;
        inputTensor.nData = 64;
        inputTensor.pData = pData;
        struct CCtx : IVisitor<const PTensor&> {
            int visit(const PTensor& t) {
                return SError::ERRORTYPE_SUCCESS;
            }
        }ctx;
        //spNet->eval(inputTensor, &ctx);
    }
}


void CNeuralNetwork::runDense() {

    SNeuralNetwork spNet = SNeuralNetwork::createDense(1);
    for(int i=0; i<100; i++) {
        PTensor inputTensor;
        inputTensor.idType = CBasicData<double>::getStaticType();
        
        int pDimSize[2] = { 2, 1 };
        double pData[2] = { 1, 2 };
        inputTensor.nDims = 2;
        inputTensor.pDimSizes = pDimSize;
        inputTensor.nData = 2;
        inputTensor.pData = pData;
        //spNet->learn(inputTensor, &ctx, nullptr);
    }
}  