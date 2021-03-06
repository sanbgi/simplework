
#include <math.h>
#include <vector>
#include <io.h>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include "CNnNetwork.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CNnNetwork.ImageNet");

static string s_rootFolder = "D:/Downloads/ILSVRC2012/subset";
static int s_nClassifies = 10;
void CNnNetwork::prepareImageNet() {
    vector<vector<string>> arrFileVecs;
    for(int i=0; i<1000; i++) {
        arrFileVecs.push_back(vector<string>());
    }
    _finddata_t folderInfo, fileInfo;
    string strRootFolder = s_rootFolder;
    intptr_t  hFolder = _findfirst((strRootFolder+"/*").c_str(), &folderInfo);
    if(hFolder != 0) {
        CTaker<intptr_t > spTaker(hFolder, [](intptr_t  h){_findclose(h);});
        do{
            if ((folderInfo.attrib &  _A_SUBDIR)) {
                string folderName = folderInfo.name;
                if(folderName != "." && folderName != "..") {
                    intptr_t  hFile = _findfirst((strRootFolder+"/"+folderName+"/*.jpeg").c_str(), &fileInfo);
                    if(hFile != 0) {
                        CTaker<intptr_t > spFileTaker(hFile, [](intptr_t  h){_findclose(h);});
                        do{
                            if( !(fileInfo.attrib & _A_SUBDIR) ) {
                                string fileName = fileInfo.name;
                                if(fileName.length() > 0){
                                    arrFileVecs[rand()%1000].push_back(folderName+"/"+fileInfo.name);
                                }
                            }
                        }while(_findnext(hFile, &fileInfo)==0);
                    }
                }
            }
        }while(_findnext(hFolder, &folderInfo)==0);
    }

    cout << "begin rand";
    vector<string> arrRandFiles;
    while(arrFileVecs.size()) {
        int iVector = rand()%arrFileVecs.size();
        vector<string>& arrFiles = arrFileVecs[iVector];
        int iRand = rand() % arrFiles.size();
        arrRandFiles.push_back(arrFiles[iRand]);

        arrFiles.erase(arrFiles.begin()+iRand);
        if(arrFiles.size()==0) {
            arrFileVecs.erase(arrFileVecs.begin()+iVector);
        }
    }

    cout << "begin write";
    ofstream w((s_rootFolder+"/images.txt").c_str());
    if(w.is_open()) {
        vector<string>::iterator it = arrRandFiles.begin();
        while(it != arrRandFiles.end()) {
            w << (*it) << "\n";
            it++;
        }
    }
}

SNnNetwork CNnNetwork::createResNet() {
    SDeviceFactory::getFactory()->setDefaultKernelDevic(SDevice::opencl());
    int pDimSizes[] = {224, 224, 3};
    SNnNetwork spNetwork = SNnNetwork::createDeviceNetwork({
        SDimension::createDimension(3,pDimSizes),
        [](const SNnVariable& spIn, SNnVariable& spOut) -> int{
            struct ResNet {
                static SNnVariable resBlock2(SNnVariable x, int n, int nUpDimension){
                    SDimension spInDimension = x.dimension();
                    int nXLayers = spInDimension.data()[spInDimension.size()-1];
                    int nLayers = nXLayers*nUpDimension;
                    while(n-->0) {
                        SNnVariable resX = x;
                        if(nXLayers != nLayers) {
                            x = x.conv({1,1,nLayers,1,2,2,"same",nullptr});
                            x = x.batchNormalize({1.0e-8});
                            nXLayers = nLayers;

                            resX = resX.conv({3,3,nLayers,1,2,2,"same"});
                            resX = resX.batchNormalize({1.0e-8});
                            resX = resX.relu();
                        }else{
                            resX = resX.conv({3,3,nLayers,1,1,1,"same"});
                            resX = resX.batchNormalize({1.0e-8});
                            resX = resX.relu();
                        }
                        resX = resX.batchNormalize({1.0e-8});
                        resX = resX.conv({3,3,nLayers,1,1,1,"same"});
                        x = x + resX;
                        x = x.relu();
                    }
                    return x;
                }
                static SNnVariable resBlock3(SNnVariable x, int n, int nUpDimension){
                    SDimension spInDimension = x.dimension();
                    int nXLayers = spInDimension.data()[spInDimension.size()-1];
                    int nLayers = nXLayers*nUpDimension;
                    while(n-->0) {
                        SNnVariable resX = x;
                        if(nXLayers != nLayers) {
                            x = x.conv({1,1,nLayers,1,2,2,"same",nullptr});
                            x = x.batchNormalize({1.0e-8});
                            nXLayers = nLayers;

                            resX = resX.conv({1,1,nLayers/4,1,2,2,"same"});
                            resX = resX.batchNormalize({1.0e-8});
                            resX = resX.relu();

                            /**
                             * ???????????????

                                //?????????????????????
                                x = x.pool({2,2,2,2});
                                x = x.linear({nLayers,false,nullptr});
                                x = x.batchNormalize({1.0e-8});
                                nXLayers = nLayers;

                                //???????????????????????????????????????????????????????????????
                                resX = x;
                                if(nXLayers != nLayers/4) {
                                    resX = resX.linear({nLayers/4,false,nullptr});
                                    resX = resX.batchNormalize({1.0e-8});
                                    resX = resX.relu();
                                }
                            * 
                            */
                        }else{
                            resX = resX.conv({1,1,nLayers/4,1,1,1,"same"});
                            resX = resX.batchNormalize({1.0e-8});
                            resX = resX.relu();
                        }
                        resX = resX.conv({3,3,nLayers/4,1,1,1,"same",nullptr});
                        resX = resX.batchNormalize({1.0e-8});
                        x = x.relu();

                        resX = resX.conv({1,1,nLayers,1,1,1, "same"});
                        resX = resX.batchNormalize({1.0e-8});
                        x = x + resX;
                        x = x.relu();
                    }
                    return x;
                }
            };
            SNnVariable x = spIn;
            x = x.conv({7,7,64,1,2,2,"same","relu"});
            x = x.maxpool({3,3,2,2,"same"});
            int nResNet = 50;
            switch(nResNet) {
            case 18:
                x = ResNet::resBlock2(x,2,1);
                x = ResNet::resBlock2(x,2,2);
                x = ResNet::resBlock2(x,2,2);
                x = ResNet::resBlock2(x,2,2);
                break;  
            case 34:
                x = ResNet::resBlock2(x,3,1);
                x = ResNet::resBlock2(x,4,2);
                x = ResNet::resBlock2(x,6,2);
                x = ResNet::resBlock2(x,3,2);
                break;  
            case 50:
                x = ResNet::resBlock3(x,3,4);
                x = ResNet::resBlock3(x,4,2);
                x = ResNet::resBlock3(x,6,2);
                x = ResNet::resBlock3(x,3,2);
                break;
            case 101:
                x = ResNet::resBlock3(x,3,4);
                x = ResNet::resBlock3(x,4,2);
                x = ResNet::resBlock3(x,23,2);
                x = ResNet::resBlock3(x,3,2);
                break;  
            case 152:
                x = ResNet::resBlock3(x,3,4);
                x = ResNet::resBlock3(x,8,2);
                x = ResNet::resBlock3(x,36,2);
                x = ResNet::resBlock3(x,3,2);
                break;  
            }
            x = x.gap();
            x = x.dense({s_nClassifies, "softmax"});
            spOut = x;
            return sCtx.success();
        }
    });
    SNnNetwork::saveFile("D://snetwork_resnet.bin", spNetwork);
    return SNnNetwork::loadFile("D://snetwork_resnet.bin");
}

static int loadImage(const SAvNetwork& spFilter, const char* szFilename, STensor& spTensor) {
    SArchivable spArchive;
    if( SArchivable::loadBinaryFile((string(szFilename)+".tbin").c_str(), spArchive) == sCtx.success() ) {
        spTensor = spArchive;
        return sCtx.success();
    }

    SAvFrame spFrame = SAvFrame::loadImageFile(szFilename);
    if( !spFrame ) {
        return sCtx.error((string("???????????????????????????????????????")+szFilename).c_str());
    }

    if( spFilter->pipeIn(spFrame, spFrame) != sCtx.success() ) {
        return sCtx.error((string("???????????????????????????????????????")+szFilename).c_str());
    }

    int pDimSizes[] = {224, 224, 3};
    if( STensor::createTensor<unsigned char>(spTensor, SDimension::createDimension(3,pDimSizes), 224 * 224 * 3 ) != sCtx.success() ) {
        return sCtx.error("??????????????????????????????");
    }

    SArchivable::saveBinaryFile((string(szFilename)+".tbin").c_str(), spTensor);
    const PAvFrame* pAvFrame = spFrame->getFramePtr();
    memcpy(spTensor.data(), pAvFrame->ppPlanes[0], 224*224*3);
    return sCtx.success();
}

void CNnNetwork::runImageNet() {
    SAvNetwork spFilter;
    SNnNetwork nn = createResNet();
    //SNnNetwork nn = SNnNetwork::loadFile("D://snetwork.bin");
    //SAvOut spWindow = SAvOut::openWindow("TestWindow", 224, 224);
    {
        PAvSample frameSample;
        frameSample.sampleType = EAvSampleType::AvSampleType_Video;
        frameSample.sampleFormat = EAvSampleFormat::AvSampleFormat_Video_RGB;
        frameSample.videoWidth = frameSample.videoHeight = 224;
        SAvFactory::getAvFactory()->openAvFrameConverter(frameSample, spFilter);
    }

    map<int,int> mapClassifies = {
        {764, 0},
        {537, 1},
        {850, 2},
        {361, 3},
        {475, 4},
        {331, 5},
        {041, 6},
        {668, 7},
        {859, 8},
        {878, 9},
    };
    string rootFolder = s_rootFolder;
    int nEpch = 40;
    while(nEpch-->0) {
        ifstream instream( (rootFolder+"/images.txt").c_str());
        ofstream outstream((rootFolder+"/errors.txt").c_str());
        if(!instream.is_open()){
            continue;
        }
        
        float sumAcc = 0;
        float sumLoss = 0;
        float sumX = 0.98f;
        int nHit = 0;
        while(!instream.eof()) {
            int nBatchs = 10;
            int nToRead = nBatchs;
            vector<string> arrFiles;
            while(!instream.eof() && nToRead-->0) {
                string filename;
                instream >> filename;
                arrFiles.push_back(filename);
            }

            if(arrFiles.size() == 0) {
                continue;
            }

            STensor spBatchIn;
            STensor spClassify;
            {
                int pDimSizes[] = {(int)arrFiles.size(), 224, 224, 3};
                if( STensor::createTensor<unsigned char>(spBatchIn, SDimension::createDimension(4,pDimSizes), 224 * 224 * 3 * (int)arrFiles.size() ) != sCtx.success() ) {
                    sCtx.error("??????????????????????????????");
                    continue;
                }

                int nFiles = (int)arrFiles.size();
                if( STensor::createTensor<int>(spClassify, SDimension::createDimension(1,&nFiles), nFiles ) != sCtx.success() ) {
                    sCtx.error("??????????????????????????????");
                    continue;
                }

                unsigned char* pBatchIn = spBatchIn.data<unsigned char>();
                int* pClassify = spClassify.data<int>();
                int nPrepared = 0;
                vector<string>::iterator itf = arrFiles.begin();
                while(itf != arrFiles.end()) {
                    STensor spImage;
                    if( loadImage(spFilter,(rootFolder+"/"+*itf).c_str(), spImage) == sCtx.success() ) {
                        memcpy(pBatchIn, spImage.data(), 224*224*3);

                        string filename = *itf;
                        size_t isepchar = filename.find('/');
                        istringstream iconverter(filename.substr(isepchar-3,3));
                        iconverter >> *pClassify;
                        *pClassify = mapClassifies[*pClassify];
                    }else{
                        cout << *itf << "\n";
                        outstream << *itf << "\n";
                        break;
                    }

                    /*
                    SAvFrame spImage = SAvFrame::loadImageFile(filename.c_str());
                    if( !spImage ) {
                        cout << *itf << "\n";
                        outstream << *itf << "\n";
                        break;
                    }
                    if( spFilter->pipeIn(spImage, spImage) == sCtx.success() ) {
                        spWindow->writeFrame(spImage);

                        const PAvFrame* pAvFrame = spImage->getFramePtr();
                        memcpy(pBatchIn, pAvFrame->ppPlanes[0], 224*224*3);

                        string filename = *itf;
                        size_t isepchar = filename.find('/');
                        istringstream iconverter(filename.substr(isepchar-3,3));
                        iconverter >> *pClassify;
                        *pClassify = mapClassifies[*pClassify];
                    }else{
                        cout << *itf << "\n";
                        outstream << *itf << "\n";
                    }*/

                    pBatchIn += 224 * 224 * 3;
                    nPrepared++;
                    pClassify++;
                    itf++;
                }
                if(nPrepared < arrFiles.size()){
                    continue;
                }

                spClassify = spClassify.oneHot(s_nClassifies, PDATATYPE_FLOAT);
                spBatchIn = spBatchIn.toFloat() * STensor::createValue<float>(1.0f/255);
            }

            //PVector sMemory;
            //spClassify->getDataInDevice(SDevice::cpu(), sMemory);
            //SDeviceFactory::getFactory()->setDefaultKernelDevic(SDevice::opencl());

            //
            // ??????????????????
            //
            STensor spOut = nn.eval(spBatchIn);
            //float* pOut = (float*)spOut.data();
            //spOut->getDataInDevice(SDevice::cpu(), sMemory);

            //
            // ???????????????
            //
            STensor spOutDeviation = spOut - spClassify;
            //spOutDeviation->getDataInDevice(SDevice::cpu(), sMemory);

            //
            // ????????????
            //
            STensor spRootMeanSquare = spOutDeviation.rootMeanSquare();
            float fRMS = *spRootMeanSquare.data<float>();
            //spOutDeviation->getDataInDevice(SDevice::cpu(), sMemory);

            
            //
            // ????????????????????????
            //
            STensor spInDeviation = nn.devia(spOut, spOutDeviation);
            float fRMS2 = *spOutDeviation.rootMeanSquare().data<float>();
            float fRMSIn = *spInDeviation.rootMeanSquare().data<float>();

            //
            // ????????????
            //
            nn.update(spInDeviation);
            

            //spClassify->getDataInDevice(SDevice::opencl(), sMemory);
            //for(int i=0; i<spClassify.size(); i++) {
            //    std::cout << spClassify.data<float>()[i];
            //}

            //
            // ????????????????????????
            //
            {
                int nOutDeviation = spOutDeviation.size();
                float* pOutDeviation = (float*)spOutDeviation.data();
                float* pOutTarget = (float*)spClassify.data();
                int nAcc = 0;
                float xAcc = 0;
                float delta = 0;
                for(int i=0; i<nOutDeviation; i++) {
                    if( pOutTarget[i] > 0.8 ) {
                        if( abs(pOutDeviation[i]) < 0.1) {
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
                if( t++ % 1 == 0)
                {
                    std::cout   << "\rt:" << t << ",\tloss:" << delta <<",\trms:"<< fRMS 
                                <<",\tnAcc:" << nAcc << ", \tavgAccDelta:" << xAcc<< "\tsAcc:"
                                << sumAcc << ",\tavgAcc:" << nHit / 10.0 / t  << "\n";
                }
            }
        }
    }
}