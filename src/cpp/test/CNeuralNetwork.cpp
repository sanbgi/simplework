
#include <math.h>
#include <vector>
#include <iostream>
#include "CNeuralNetwork.h"

using namespace sw;
using namespace sw::av;
using namespace sw::math;
using namespace sw::nn;


void CNeuralNetwork::run() {

    STensor images = SNeuralNetwork::loadIdxFile("D:\\Workspace\\simplework\\mnist\\train-images.gz");
    STensor labels = SNeuralNetwork::loadIdxFile("D:\\Workspace\\simplework\\mnist\\train-labels.gz");
    SNeuralNetwork nn = createNetwork();
    STensor spImageDimVector = images->getDimVector();
    STensor spLabelDimVector = labels->getDimVector();
    if( images && labels && spImageDimVector->getDataSize() == 3 && spLabelDimVector->getDataSize() == 1 && 
        spImageDimVector->getDataAt<int>(0) == spLabelDimVector->getDataAt<int>(0) ) {
        
        int* pImageDimSizes = spImageDimVector->getDataPtr<int>();
        int nAllImages = pImageDimSizes[0];
        int nBatchImages = 10;
        int nImageSize = pImageDimSizes[1] * pImageDimSizes[2];

        int pDimSizes[3] = {nBatchImages, pImageDimSizes[1], pImageDimSizes[2] };
        STensor spInDimVector;
        STensor::createVector(spInDimVector, 3, pDimSizes);
        for( int iImage = 0; iImage<nAllImages; /*iImage+=nBatchImages*/ ) {

            unsigned char* pImageData = images->getDataPtr<unsigned char>() + iImage * nImageSize;
            int nImages = nBatchImages > (nAllImages - iImage) ? (nAllImages - iImage) : nBatchImages;
            if(nImages < nBatchImages) {
                break;
            }

            int nData = nImages * nImageSize;
            CTaker<double*> spData(new double[nData], [](double* ptr) {
                delete[] ptr;
            });
            double* pData = spData;
            for( int j=0; j<nData; j++) {
                pData[j] = *(pImageData + j) / 256.0;
            }

            STensor spIn;
            STensor::createTensor(spIn, spInDimVector, nData, pData);

            STensor spOut;
            nn->eval(spIn, spOut);

            STensor spOutDeviation;
            STensor& spDimOutVector = spOut->getDimVector();
            STensor::createTensor(spOutDeviation, spDimOutVector, spOut->getDataType(), spOut->getDataSize());
            {
                unsigned char* pKind = labels->getDataPtr<unsigned char>() + iImage;
                int* pOutDimSizes = spDimOutVector->getDataPtr<int>();
                double delta = 0;
                int nTensorSize = pOutDimSizes[1];
                int nAcc = 0;
                double xAcc = 0;
                for( int iTensor=0; iTensor<pOutDimSizes[0]; iTensor++) {
                    int iKind = pKind[iTensor];
                    double* pOutputArray = spOut->getDataPtr<double>() + iTensor * nTensorSize;
                    double* pDeviationArray = spOutDeviation->getDataPtr<double>() + iTensor * nTensorSize;
                    for(int i=0; i<nTensorSize; i++) {
                        if( i==iKind) {
                            pDeviationArray[i] = pOutputArray[i] - 1;
                            xAcc += (1-pOutputArray[i])/10;
                            if(1-pOutputArray[i] < 0.1) {
                                nAcc++;
                            }
                        }else{
                            pDeviationArray[i] = pOutputArray[i];
                        }
                        delta += pDeviationArray[i] * pDeviationArray[i];
                    }
                }
                static int t = 0;
                if( t++ % 20 == 0) {
                    std::cout << "\rtrain:" << t << ", delta :" << delta <<", nAcc:" << nAcc << ", xAcc:" << xAcc<< "\n";
                }
            }

            STensor spInDeviation;
            nn->learn(spOut, spOutDeviation, spIn, spInDeviation);
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
        inputTensor.idType = CBasicType<double>::getThisType();
        
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
        inputTensor.idType = CBasicType<double>::getThisType();
        
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
        inputTensor.idType = CBasicType<double>::getThisType();
        
        int pDimSize[2] = { 2, 1 };
        double pData[2] = { 1, 2 };
        inputTensor.nDims = 2;
        inputTensor.pDimSizes = pDimSize;
        inputTensor.nData = 2;
        inputTensor.pData = pData;
        struct CLearnCtx : public SNeuralNetwork::ILearnCtx {
                int getOutputDeviation(const STensor& outputTensor, STensor& outputDeviation) {
                    //outputDeviation.pDoubleArray[0] =  outputTensor.pDoubleArray[0] - (0.7 * 1 + 0.3);
                    //outputDeviation.pDoubleArray[1] =  outputTensor.pDoubleArray[1] - (0.7 * 2 + 0.3);
                    return SError::ERRORTYPE_SUCCESS;
                }
                unsigned char* pKind;
        }ctx;
        //spNet->learn(inputTensor, &ctx, nullptr);
    }
}  