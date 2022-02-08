#include <iostream>
#include <vector>

#include "../inc/SimpleWork.h"
#include <math.h>
#include "CNnNetwork.h"
#include "CAvNetwork.h"

using namespace sw;
using namespace sw;
using namespace sw;
using namespace sw;
static SCtx sCtx("TestSimpleWork");

void testIdx() {
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

    //FAA a = { 10 };
    //FBB b = { 20 };
    //fun(CData<FAA>(a));
    //fun(CData<FBB>(b));
    //unsigned int la = SData::getBasicTypeIdentifier<int>();
    //unsigned long lb = SData::getTypeIdentifier("aaaaaaaaaaaaaaaafadfasdf");
    //unsigned long lc = SData::getTypeIdentifier("ffffffffffffffffasdfasdfasdfasdfa");
    //unsigned long ld = SData::getTypeIdentifier("ffffffffffffffffffasdfasdfadsfadsfasdf");
    //testWriteFile();
    //testPipe();
    //testTensor();
    //testNN();
    //testIdx();
    //CNnNetwork::runFile();
    CNnNetwork::run();
    //CAvNetwork::runImage();

    /*
    int i=10;
    CData s(i);
    CData * pData = new CData(i);
    fun(s);
    fun(CData(i));
    fun(CData(20));
    const int* pPtr = s.getPtr<int>();
    //fun(CData(10));
    */
    return 0;
}
