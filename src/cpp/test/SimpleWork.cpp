#include <iostream>
#include <vector>

#include "../inc/SimpleWork.h"
#include <math.h>
#include "CNnNetwork.h"
#include "CAvNetwork.h"

using namespace sw;


void testIdx() {
    SCtx sCtx("TestSimpleWork");

    //std::string strFilename = "D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-labels-idx1-ubyte965g-S4A7G3r0jpgiMMdvSNx7KP_oN7677JZkkIErsY.gz";
    std::string strFilename = "D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-images-idx3-ubytejUIsewocHHkkWlvPB_6G4z7q_ueSuEWErsJ29aLbxOY.gz";
    //SNnNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-images-idx3-ubytejUIsewocHHkkWlvPB_6G4z7q_ueSuEWErsJ29aLbxOY.gz", spTensor);
    //SNnNetwork::getFactory()->readIdxFile("", spTensor);
    //SNnNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_train-images-idx3-ubyteRA_Kv3PMVG-iFHXoHqNwJlYF9WviEKQCTSyo8gNSNgk.gz", spTensor);
    //SNnNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_train-labels-idx1-ubyteNVJTSgpVi77WrtMrMMSVzKI9Vn7FLKyL4aBzDoAQJVw.gz", spTensor);
    SNnPipe spReader;
    SNnFactory::getFactory()->openIdxFileReader(strFilename.c_str(), spReader);
    if(spReader) {
        int nRead = 10;
        int nReaded = 0;
        STensor spIn;
        STensor::createVector(spIn, 1, &nRead);
        STensor spOut;
        while(spReader->push(spIn, spOut) == sCtx.success()) {
            nReaded += 10;
            //std::cout << spOut->getDataSize();
        }

        nRead = 0;
    }
}


int main(int argc, char *argv[]){

    //SDevice cuda = SDevice::cuda();
    /*
    std::vector<SDeviceMemory> arrMemorirs;
    SDevice spDevice = SDevice::opencl();
    int i=0;
    int nElementSize = 1000000000;
    SDeviceMemory spMemory = SDeviceMemory::createDeviceMemory(SDevice::opencl(), 1000000);
    for( i=0; i<10000; i++) {
        SMathKernel::equal<int>(spDevice,spMemory.data(spDevice),0, i, spMemory.size()/sizeof(int));
        if( !spMemory ) {
            break;
        }

        int v[1000];
        spMemory->readMemory({100,v});

        if(v[0] != i) {
            break;
        }
    }
    std::cout << "max i:" << i;
    */


    /*
    STensor sTestTensor = STensor::createVector<int>(100000);

    SDevice opencl = SDevice::opencl();

    void* pData = sTestTensor.data(opencl);
    opencl->runKernel({nullptr, "sw.math.TensorAssign", "intEval"}, 
            1, std::vector<PMemory>({{sizeof(void*), &pData}}).data(), 
            1, std::vector<int>({100000}).data());

    int* pV = sTestTensor.data<int>();

    for(int i=0; i<100000; i++) {
        if(pV[i] != i) {
            std::cout << pV[i] << "--" << i << "found";
        }
    }*/

    //SDimension(3,{0,1,2});
    //testOpenCL();
    //cl::Buffer bf();
    //cl::copy();
    //FAA a = { 10 };
    //FBB b = { 20 };
    //fun(CData<FAA>(a));
    //fun(CData<FBB>(b));
    //unsigned int la = SData::getBasicTypeIdentifier<int>();
    //unsigned long lb = SData::getDataType("aaaaaaaaaaaaaaaafadfasdf");
    //unsigned long lc = SData::getDataType("ffffffffffffffffasdfasdfasdfasdfa");
    //unsigned long ld = SData::getDataType("ffffffffffffffffffasdfasdfadsfadsfasdf");
    //testWriteFile();
    //testPipe();
    //testTensor();
    //testNN();
    //testIdx();
    //CNnNetwork::runFile();
    CNnNetwork::run();
    //CAvNetwork::run();

    return 0;
}
