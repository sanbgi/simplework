
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
    long hFolder = _findfirst((strRootFolder+"/*").c_str(), &folderInfo);
    if(hFolder != 0) {
        CTaker<long> spTaker(hFolder, [](long h){_findclose(h);});
        do{
            if ((folderInfo.attrib &  _A_SUBDIR)) {
                string folderName = folderInfo.name;
                if(folderName != "." && folderName != "..") {
                    long hFile = _findfirst((strRootFolder+"/"+folderName+"/*.jpeg").c_str(), &fileInfo);
                    if(hFile != 0) {
                        CTaker<long> spFileTaker(hFile, [](long h){_findclose(h);});
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
    int pDimSizes[] = {224, 224, 3};
    SNnNetwork spNetwork = SNnNetwork::createOpenCLNetwork({
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
                             * 个人推荐，

                                //池化层缩减尺寸
                                x = x.pool({2,2,2,2});
                                x = x.linear({nLayers,false,nullptr});
                                x = x.batchNormalize({1.0e-8});
                                nXLayers = nLayers;

                                //如果有缩减层数需求，则通过线性变化缩减尺寸
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


void CNnNetwork::runImageNet() {
    //prepareImageNet();
    //return;

    SAvNetwork spFilter;
    //SNnNetwork nn = createResNet();
    SNnNetwork nn = SNnNetwork::loadFile("D://snetwork.bin");
    SAvOut spWindow = SAvOut::openWindow("TestWindow", 224, 224);
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
    int nEpch = 10;
    while(nEpch-->0) {
        ifstream instream( (rootFolder+"/images.txt").c_str());
        ofstream outstream((rootFolder+"/errors.txt").c_str());
        if(!instream.is_open()){
            continue;
        }
        
        float sumAcc = 0;
        float sumLoss = 0;
        float sumX = 0.98;
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
                if( STensor::createTensor<unsigned char>(spBatchIn, SDimension::createDimension(4,pDimSizes), 224 * 224 * 3 * arrFiles.size() ) != sCtx.success() ) {
                    sCtx.error("创建输入图片张量失败");
                    continue;
                }

                int nFiles = arrFiles.size();
                if( STensor::createTensor<int>(spClassify, SDimension::createDimension(1,&nFiles), nFiles ) != sCtx.success() ) {
                    sCtx.error("创建输入图片张量失败");
                    continue;
                }

                unsigned char* pBatchIn = spBatchIn.data<unsigned char>();
                int* pClassify = spClassify.data<int>();
                int nPrepared = 0;
                vector<string>::iterator itf = arrFiles.begin();
                while(itf != arrFiles.end()) {
                    SAvFrame spImage = SAvFrame::loadImageFile((rootFolder+"/"+*itf).c_str());
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
                    }
                    pBatchIn += 224 * 224 * 3;
                    nPrepared++;
                    pClassify++;
                    itf++;
                }
                if(nPrepared < arrFiles.size()){
                    continue;
                }

                spClassify = SNnNetwork::classifyTensor(s_nClassifies, spClassify);
                spBatchIn = SNnNetwork::normalizeTensor(spBatchIn);
            }

            //
            // 神经网络求解
            //
            STensor spOut = nn.eval(spBatchIn);

            //
            // 计算偏差量
            //
            STensor spOutDeviation = spOut - spClassify;

            //
            // 求均方根
            //
            STensor spRootMeanSquare = spOutDeviation.rootMeanSquare();
            float fRMS = *spRootMeanSquare.data<float>();
            if( fRMS > 0.0001 ) {
                //
                // 学习更新神经网络
                //
                STensor spInDeviation = nn.devia(spOut, spOutDeviation);

                //
                // 更新网络
                //
                nn.update(spInDeviation);

                //
                // 保存网络
                //
                SNnNetwork::saveFile("D://snetwork.bin", nn);
            }


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
                if( t++ % 10 == 0)
                {
                    std::cout   << "\rt:" << t << ",\tloss:" << delta <<",\trms:"<< fRMS 
                                <<",\tnAcc:" << nAcc << ", \tavgAccDelta:" << xAcc<< "\tsAcc:"
                                << sumAcc << ",\tavgAcc:" << nHit / 10.0 / t  << "\n";
                }
            }
        }
    }
}