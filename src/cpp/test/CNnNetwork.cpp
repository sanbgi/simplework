
#include <math.h>
#include <vector>
#include <iostream>
#include "CNnNetwork.h"

using namespace sw;

static SCtx sCtx("CNnNetwork.Test");

void CNnNetwork::run() {
    runLearn();
    //runTest();
}

void CNnNetwork::runTestNetwork(){
}

void CNnNetwork::runLearn() {
    //
    // 一次读取10个
    //
    STensor spPipeIn = STensor::createValue(10);
    //SNnNetwork nn = createLayerNetwork();
    //SNnNetwork nn = createRnnNetwork();
    //SNnNetwork nn = createNetwork();
    //SNnNetwork nn = createGlobalPollNetwork();
    //SNnNetwork nn = createUnitNetwork();
    //SNnNetwork nn = createLeNet();
    SNnNetwork nn = createResNetwork();
    //SNnNetwork nn = createShiftNetwork();
    //SNnNetwork nn = SNnNetwork::loadFile("D://snetwork.bin");
    int nLoops = 20;
    while(nLoops-->0)
    {
        SNnPipe spImageReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\train-images.gz");
        SNnPipe spLabelReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\train-labels.gz");

        float sumAcc = 0;
        float sumLoss = 0;
        float sumX = 0.98;
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
            STensor spInDeviation = nn.devia(spOut, spOut - spClassify);

            //
            // 更新网络
            //
            nn.update(spInDeviation);

            //
            // 打印一些结果信息
            //
            {
                int nOutDeviation = spOutDeviation->getDataSize();
                float* pOutDeviation = spOutDeviation->getDataPtr<float>();
                float* pOutTarget = spClassify->getDataPtr<float>();
                int nAcc = 0;
                float xAcc = 0;
                float delta = 0;
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
        SNnNetwork::saveFile("D://snetwork.bin", nn);
    }
}

void CNnNetwork::runTest() {
    SNnPipe spImageReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\t10k-images.gz");
    SNnPipe spLabelReader = SNnNetwork::openIdxFileReader("D:\\Workspace\\simplework\\mnist\\t10k-labels.gz");

    //
    // 一次读取10个
    //
    STensor spPipeIn = STensor::createValue(10);
    SNnNetwork nn = createRnnNetwork();
    //SNnNetwork nn = createLayerNetwork();
    //SNnNetwork nn = createTestNetwork();
    //SNnNetwork nn = createNetwork();
    //SNnNetwork nn = createRotNetwork();
    //SNnNetwork nn = createShiftNetwork();
    //SNnNetwork nn = SNnNetwork::loadFile("D://snetwork.bin");

    float sumAcc = 0;
    float sumLoss = 0;
    float sumX = 0.98;
    int nHit = 0;
    int nData = 0;

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
            nData += 10;    
            int nOutDeviation = spOutDeviation->getDataSize();
            float* pOutDeviation = spOutDeviation->getDataPtr<float>();
            float* pOutTarget = spClassify->getDataPtr<float>();
            int nAcc = 0;
            float xAcc = 0;
            float delta = 0;
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
                            << sumAcc << ",\tavgAcc:" << nHit * 1.0 / nData  << "\n";
            }
        }
    }
}


SNnNetwork CNnNetwork::createTestNetwork() {
    std::vector<SNnNetwork> arrNets;
    /*
    arrNets.push_back(SNnNetwork::createConv(3,3,6,"same"));
    arrNets.push_back(SNnNetwork::createConv(3,3,6,"same"));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createConv(3,3,16));
    arrNets.push_back(SNnNetwork::createConv(3,3,16));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createConv(5,5,120));
    //arrNets.push_back(SNnNetwork::createConv(3,3,120));
    arrNets.push_back(SNnNetwork::createDense(84));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    
    return SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    */
    return SNnNetwork();
}

SNnNetwork CNnNetwork::createLayerNetwork() {
    int pDimSizes[] = {28, 28};
    SNnNetwork spNetwork = SNnNetwork::createNetwork({
        SDimension::createDimension(2,pDimSizes),
        [](const SNnVariable& spIn, SNnVariable& spOut) -> int{
            SNnVariable x = spIn;
            x = x.conv({5,5,32});
            x = x.pool({2,2,2,2});
            x = x.batchNormalize(PNnBatchNormalize());
            x = x.conv({7,7,64});
            x = x.pool({2,2,2,2});
            x = x.batchNormalize(PNnBatchNormalize());
            x = x.dense({576});
            x = x.dense({10, "softmax"});
            spOut = x;
            return sCtx.success();
        }
    });
    SNnNetwork::saveFile("D://snetwork.bin", spNetwork);
    /*
    std::vector<SNnUnit> arrUnits;
    arrUnits.push_back(SNnUnit::createConvUnit({5,5,32}));
    arrUnits.push_back(SNnUnit::createPoolUnit({2,2,2,2}));
    arrUnits.push_back(SNnUnit::createBatchNormalizeUnit({1.0e-8}));
    arrUnits.push_back(SNnUnit::createConvUnit({7,7,64}));
    arrUnits.push_back(SNnUnit::createPoolUnit({2,2,2,2}));
    arrUnits.push_back(SNnUnit::createBatchNormalizeUnit({1.0e-8}));
    arrUnits.push_back(SNnUnit::createDenseUnit({576}));
    arrUnits.push_back(SNnUnit::createDenseUnit({10, "softmax"}));

    int pDimSizes[] = {28, 28};
    SDimension spDim = SDimension::createDimension(2,pDimSizes);
    SNnNetwork spNet = SNnNetwork::createNetwork(SNnUnit::createSequenceUnit(arrUnits.size(), arrUnits.data()),spDim);
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    */
    return SNnNetwork::loadFile("D://snetwork.bin");
}

SNnNetwork CNnNetwork::createRnnNetwork() {
    /*
    std::vector<SNnNetwork> arrNets;
    arrNets.push_back(SNnNetwork::createRnn(50,true));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    return SNnNetwork::createSequence(arrNets.size(), arrNets.data());

    std::vector<SNnLayer> arrUnits;
    arrUnits.push_back(SNnLayer::createGruLayer(50, "sequence"));
    arrUnits.push_back(SNnLayer::createDenseLayer({10, "softmax"}));
    int pDimSizes[] = {28, 28};
    SDimension spDim = SDimension::createDimension(2,pDimSizes);
    SNnNetwork spNet =  SNnNetwork::createNetwork(arrUnits.size(),arrUnits.data(),spDim);
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");
    */
    return SNnNetwork();
}

SNnNetwork CNnNetwork::createNetwork() {
    /*
    std::vector<SNnNetwork> arrNets;
    arrNets.push_back(SNnNetwork::createConv(5,5,32));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createConv(7,7,64));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createDense(576));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");*/
    return SNnNetwork();
}

SNnNetwork CNnNetwork::createShiftNetwork() {
    std::vector<SNnNetwork> arrNets;
    /*
    arrNets.push_back(SNnNetwork::createShiftConv(5,5,8,8,"same"));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createShiftConv(7,7,32,8));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createDense(576));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");*/
    return SNnNetwork();
}

SNnNetwork CNnNetwork::createResNetwork() {
    int pDimSizes[] = {28, 28};
    SNnNetwork spNetwork = SNnNetwork::createNetwork({
        SDimension::createDimension(2,pDimSizes),
        [](const SNnVariable& spIn, SNnVariable& spOut) -> int{

            struct ResNet {
                static SNnVariable resBlock2(SNnVariable x, int n, int nUpDimension=2){
                    SDimension spInDimension = x.dimension();
                    int nXLayers = spInDimension.data()[spInDimension.size()-1];
                    int nLayers = nXLayers*nUpDimension;
                    while(n-->0) {
                        SNnVariable resX = x;
                        if(nXLayers != nLayers) {
                            x = x.pool({2,2,2,2,"same"});
                            x = x.conv({3,3,nLayers,1,1,1,"same","relu"});
                            //resX = resX.conv({3,3,nLayers,1,2,2,"same",nullptr});
                            nXLayers = nLayers;
                        }else{
                            //resX = resX.batchNormalize({1.0e-8});
                            //resX = resX.relu();
                            //resX = resX.conv({3,3,nLayers,1,1,1,"same",nullptr});
                        }
                        //resX = resX.batchNormalize({1.0e-8});
                        //resX = resX.relu();
                        //resX = resX.conv({3,3,nLayers,1,1,1,"same",nullptr});
                        //x = x + resX;
                    }
                    return x;
                }
            };

            SNnVariable x = spIn;
            x = x.conv({5,5,32,1,1,1,"same"});
            x = ResNet::resBlock2(x,1);
            x = ResNet::resBlock2(x,1);
            x = ResNet::resBlock2(x,1);
            x = x.gap();
            x = x.dense({10, "softmax"});
            spOut = x;
            return sCtx.success();
        }
    });
    SNnNetwork::saveFile("D://snetwork.bin", spNetwork);
    return SNnNetwork::loadFile("D://snetwork.bin");
}

SNnNetwork CNnNetwork::createGlobalPollNetwork() {
    std::vector<SNnNetwork> arrNets;
    /*
    arrNets.push_back(SNnNetwork::createShiftConv(3,3,4,8,"same"));
    arrNets.push_back(SNnNetwork::createShiftConv(3,3,16,1,"same"));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createShiftConv(3,3,32,2,"same"));
    arrNets.push_back(SNnNetwork::createShiftConv(3,3,64,1,"same"));
    arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    arrNets.push_back(SNnNetwork::createShiftConv(3,3,128,1));
    //arrNets.push_back(SNnNetwork::createConv(3,3,10));
    //arrNets.push_back(SNnNetwork::createPool(2,2,2,2));
    //arrNets.push_back(SNnNetwork::createConv(3,3,10));
    //arrNets.push_back(SNnNetwork::createGlobalPool(nullptr, "softmax"));
    arrNets.push_back(SNnNetwork::createDense(10, 0, "softmax"));
    SNnNetwork spNet = SNnNetwork::createSequence(arrNets.size(), arrNets.data());
    
    SNnNetwork::saveFile("D://snetwork.bin", spNet);
    return SNnNetwork::loadFile("D://snetwork.bin");*/
    return SNnNetwork();
}

SNnNetwork CNnNetwork::createLeNet(){
    int pDimSizes[] = {28, 28};
    SNnNetwork spNetwork = SNnNetwork::createNetwork({
        SDimension::createDimension(2,pDimSizes),
        [](const SNnVariable& spIn, SNnVariable& spOut) -> int{
            SNnVariable x = spIn;
            x = x.conv({5,5,6,1,1,1,"same", nullptr});
            x = x.pool({2,2,2,2});
            x = x.relu();
            x = x.conv({5,5,16,1,1,1,nullptr,nullptr});
            x = x.pool({2,2,2,2});
            x = x.relu();
            x = x.dense({120, "relu"});
            x = x.dense({84, "relu"});
            x = x.dense({10, "softmax"});
            spOut = x;
            return sCtx.success();
        }
    });
    spNetwork = SNnNetwork::createNetwork(SDimension::createDimension(2,pDimSizes), SObject::createObject("sw.nn.LeNetModule"));
    SNnNetwork::saveFile("D://snetwork.bin", spNetwork);
    return SNnNetwork::loadFile("D://snetwork.bin");
}
