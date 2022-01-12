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
SIMPLEWORK_INTERFACECLASS_ENTER(MyObject, "TestSimpleWork.MyObject")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IMyObject", 211202)
        virtual void sayHi() = 0;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    void sayHi() {
        if( getPtr() ) {
            getPtr()->sayHi();
        }
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(MyObject)

class CMyObject : public CObject, public IMyObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IMyObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    void sayHi() {
        std::cout << "Great ! Hi everyone!";
    }
};
SIMPLEWORK_FACTORY_AUTO_REGISTER(CMyObject, SMyObject::__getClassKey())

struct FAA {
    SIMPLEWORK_PDATAKEY(FAA, "FAA")

    int i;
};

struct FBB {
    SIMPLEWORK_PDATAKEY(FBB, FAA::__getClassKey())

    double i; 
};

void fun(const PData& r) {
    CData<FAA> ir(r);
    if(ir.isThisType()) {
        const FAA* pInt = ir.getDataPtr();
        std::cout << "int found:  ";
    }
    CData<FBB> dr(r);
    if(dr.isThisType()) {
        const FBB* pDouble = dr.getDataPtr();
        std::cout << "double found:  ";
    }

    int i=10;
    int aa[i];
    aa[0] = i;
}


int testPipe() {
    /*
    SPipe avIn;
    if( SAvFactory::getAvFactory()->openAvFileReader("d:/tt.mkv", avIn) != sCtx.success() ) {
        return sCtx.error();
    }

    SPipe avWindows;
    if( SAvFactory::getAvFactory()->openWindow("DisplayAv", 640, 360, avWindows) != sCtx.success() ){
        return sCtx.error();
    } 

    SPipe avSpeaker;
    if( SAvFactory::getAvFactory()->openSpeaker(nullptr, avSpeaker) ) {
        return sCtx.error();
    }

    SPipe avFileWriter;
    if( SAvFactory::getAvFactory()->openAvFileWriter("d:/tt2.mkv", avFileWriter) ) {
        return sCtx.error();
    }

    SPipe pipes[3] = { avWindows, avSpeaker, avFileWriter };
    SPipe avOut;
    if( SAvFactory::getAvFactory()->createParallelPipe(3, pipes, avOut) != sCtx.success() ) {
        return sCtx.error();
    }

    SPipe inPipe[2] = {avIn, avOut};
    SPipe av;
    if( SAvFactory::getAvFactory()->createSequencePipe(2, inPipe, av) != sCtx.success() ) {
        return sCtx.error();
    }

    int nFrames=0;
    while(av->pushData(CData<PAvFrame>(nullptr), nullptr) == sCtx.success()) {
        std::cout << "frames: " << ++nFrames << "processed \n";
    }*/

    return sCtx.success();
}

void testNN() {
    /*
    SNnNetwork n;
    SNnNetwork::getFactory()->createDense(1, SNnNetwork::ACTIVATION_ReLU, n);
    double v = 0.5;
    int dimsize[] = { 1 };
    PTensor inputTensor;
    inputTensor.idType = SData::getTypeIdentifier<CBasicData<double>>();
    inputTensor.nData = 1;
    inputTensor.pData = &v;
    inputTensor.nDims = 1;
    inputTensor.pDimSizes = dimsize;
    struct LearnCtx : public SNnNetwork::ILearnCtx {
        int getOutputDeviation(const PTensor& outputTensor, PTensor& outputDeivation) {
            double y = 0.7*(*pV) - 0.3;
            *outputDeivation.pDoubleArray = outputTensor.pDoubleArray[0] - 1/(1+exp(-y));
            return sCtx.success();
        }
        double *pV;
    }ctx;
    ctx.pV = &v;

    for( int i=1; i<1000; i++) {
        v = rand() % 100 / 50.0;
        n->learn(inputTensor, &ctx, nullptr);
    }*/
}

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
    CNnNetwork::runFile();
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
