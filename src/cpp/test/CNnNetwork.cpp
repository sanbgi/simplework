
#include <math.h>
#include <vector>
#include <iostream>
#include "CNnNetwork.h"

using namespace sw;
using namespace sw;
using namespace sw;
using namespace sw;
using namespace sw;

static SCtx sCtx("CNnNetwork.Test");

void CNnNetwork::run() {
    runLearn();
}

void CNnNetwork::runLearn() {

    SNnPipe spImageReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\train-images.gz");
    SNnPipe spLabelReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\train-labels.gz");

    //
    // 一次读取10个
    //
    STensor spPipeIn = STensor::createValue(10);
    SNnNetwork nn = createNetwork();
    //SNnNetwork nn = createRotNetwork();
    //SNnNetwork nn = createShiftNetwork();
    //SNnNetwork nn = SNnNetwork::loadFile("D://snetwork.bin")

    double sumAcc = 0;
    double sumLoss = 0;
    double sumX = 0.98;
    int nHit = 0;

    STensor spBatchImage, spBatchLabel;
    while( spImageReader->push(spPipeIn, spBatchImage) == sCtx.success() && spLabelReader->push(spPipeIn, spBatchLabel) == sCtx.success() ) {
        // 
        // 分类信息
        //
        STensor spClassify = SNnNetwork::classifyTensor(10, spBatchLabel);

        //
        // 图片信息，将字节类型图片张量，转化为[0,1)浮点类型张量
        //
        STensor spIn = SNnNetwork::normalizeTensor(spBatchImage);

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
                if( pOutTarget[i] > 0.8 ) {
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

    //SNnNetwork::saveFile("D://snetwork.bin", nn);
}

void CNnNetwork::runTest() {
    SNnPipe spImageReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\t10k-images.gz");
    SNnPipe spLabelReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\t10k-labels.gz");

    //
    // 一次读取10个
    //
    STensor spPipeIn = STensor::createValue(10);
    SNnNetwork nn = createNetwork();
    //SNnNetwork nn = createRotNetwork();
    //SNnNetwork nn = createShiftNetwork();
    //SNnNetwork nn = SNnNetwork::loadFile("D://snetwork.bin");

    double sumAcc = 0;
    double sumLoss = 0;
    double sumX = 0.98;
    int nHit = 0;

    STensor spBatchImage, spBatchLabel;
    while( spImageReader->push(spPipeIn, spBatchImage) == sCtx.success() && spLabelReader->push(spPipeIn, spBatchLabel) == sCtx.success() ) {
        // 
        // 分类信息
        //
        STensor spClassify = SNnNetwork::classifyTensor(10, spBatchLabel);

        //
        // 图片信息，将字节类型图片张量，转化为[0,1)浮点类型张量
        //
        STensor spIn = SNnNetwork::normalizeTensor(spBatchImage);

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
        //nn.learn(spOut, spOutDeviation);

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
                if( pOutTarget[i] > 0.8 ) {
                    if(pOutDeviation[i] > -0.6) {
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


SNnNetwork CNnNetwork::createNetwork() {
    std::vector<SNnNetwork> arrNets;
    arrNets.push_back(SNnNetwork::createConv(5,5,32));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createConv(7,7,64));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createDense(576));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");
}

SNnNetwork CNnNetwork::createShiftNetwork() {
    std::vector<SNnNetwork> arrNets;
    arrNets.push_back(SNnNetwork::createShiftConv(5,5,8,8,"same"));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createShiftConv(7,7,32,8));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createDense(576));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");
}

SNnNetwork CNnNetwork::createRotNetwork() {
    std::vector<SNnNetwork> arrNets;
    arrNets.push_back(SNnNetwork::createRotConv(7,7,8,0,20));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createConv(7,7,64));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createDense(576));
    //arrNets.push_back(SNnNetwork::createDense(10));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");
}

SNnNetwork CNnNetwork::createGlobalPollNetwork() {
    std::vector<SNnNetwork> arrNets;
    arrNets.push_back(SNnNetwork::createConv(5,5,32));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createConv(7,7,64));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createDense(576));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");
}

void CNnNetwork::runFile() {

    SNnNetwork spNet = SNnNetwork::createDense(576);
    SIoFactory::getFactory()->saveArchive("D://Dense.bin", spNet);

    spNet.release();

    SIoArchivable arObj; 
    SIoFactory::getFactory()->loadArchive("D://Dense.bin", arObj);
    spNet = arObj;
    spNet.release();
}