
#include <math.h>
#include <vector>
#include <iostream>
#include "CNeuralNetwork.h"

using namespace sw;
using namespace sw::av;
using namespace sw::math;
using namespace sw::nn;

void CNeuralNetwork::run() {
    SData images = SNeuralNetwork::loadIdxFile("D:\\Workspace\\simplework\\mnist\\train-images.gz");
    SData labels = SNeuralNetwork::loadIdxFile("D:\\Workspace\\simplework\\mnist\\train-labels.gz");
    SNeuralNetwork nn = createNetwork();
    const PTensor* pImages = images.getDataPtr<PTensor>();
    const PTensor* pLabels = labels.getDataPtr<PTensor>();
    if( pImages && pLabels && pImages->nDims == 3 && pLabels->nDims == 1 && 
        pImages->pDimSizes[0] == pLabels->pDimSizes[0] ) {
        int nImages = pImages->pDimSizes[0];
        for(int i=0; i<nImages; i++) {
            PTensor tensorImage = *pImages;
            tensorImage.nDims--;
            tensorImage.pDimSizes++;
            tensorImage.nData = pImages->nData / nImages;
            int nData = pImages->nData/nImages;
            double pData[nData];
            for( int j=0; j<nData; j++) {
                pData[j] = *(tensorImage.pByteArray + nData + j) / 256.0;
            }
            tensorImage.pData = pData;

            struct CLearnCtx : public SNeuralNetwork::ILearnCtx {
                int getOutputDeviation(const PTensor& outputTensor, PTensor& outputDeviation) {
                    double delta = 0;
                    double outputData[10];
                    for(int i=0; i<outputTensor.nData; i++) {
                        if( i== iKind) {
                            outputDeviation.pDoubleArray[i] = 1 - outputTensor.pDoubleArray[i];
                        }else{
                            outputDeviation.pDoubleArray[i] = 0 - outputTensor.pDoubleArray[i];
                        }
                        outputData[i] = outputTensor.pDoubleArray[i];
                        delta += outputDeviation.pDoubleArray[i] * outputDeviation.pDoubleArray[i];
                    }
                    static int t = 0;
                    if( t++ % 100 == 0) {
                        std::cout << "\rtrain:" << t << ", delta :" << delta;
                    }
                    return SError::ERRORTYPE_SUCCESS;
                }
                int iKind;
            }ctx;
            ctx.iKind = pLabels->pByteArray[i];
            nn->learn(tensorImage, &ctx, nullptr);
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