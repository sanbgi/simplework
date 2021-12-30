#include <iostream>
#include <vector>

#include "../inc/SimpleWork.h"
#include <math.h>

using namespace sw;
using namespace sw::av;
using namespace sw::math;
using namespace sw::nn;
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
SIMPLEWORK_FACTORY_REGISTER(CMyObject, SMyObject::getClassKey())

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
    SPipe avIn;
    if( SAvFactory::getAvFactory()->openAvFileReader("d:/tt.mkv", avIn) != sCtx.Success() ) {
        return sCtx.Error();
    }

    SPipe avWindows;
    if( SAvFactory::getAvFactory()->openWindow("DisplayAv", 640, 360, avWindows) != sCtx.Success() ){
        return sCtx.Error();
    } 

    SPipe avSpeaker;
    if( SAvFactory::getAvFactory()->openSpeaker(nullptr, avSpeaker) ) {
        return sCtx.Error();
    }

    SPipe avFileWriter;
    if( SAvFactory::getAvFactory()->openAvFileWriter("d:/tt2.mkv", avFileWriter) ) {
        return sCtx.Error();
    }

    SPipe pipes[3] = { avWindows, avSpeaker, avFileWriter };
    SPipe avOut;
    if( SAvFactory::getAvFactory()->createParallelPipe(3, pipes, avOut) != sCtx.Success() ) {
        return sCtx.Error();
    }

    SPipe inPipe[2] = {avIn, avOut};
    SPipe av;
    if( SAvFactory::getAvFactory()->createSequencePipe(2, inPipe, av) != sCtx.Success() ) {
        return sCtx.Error();
    }

    int nFrames=0;
    while(av->pushData(CData<PAvFrame>(nullptr), nullptr) == sCtx.Success()) {
        std::cout << "frames: " << ++nFrames << "processed \n";
    }

    return sCtx.Success();
}

void testTensor() {

    int dim1[] = {2, 3};
    double data1[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    PTensor t1 = {
        SData::getTypeIdentifier<CBasicType<double>>(),
        2,
        dim1,
        6,
        data1
    };

    int dim2[] = {3, 1};
    double data2[] = { 6.0, 7.0, 8.0, 9.0, 10.0, 11.0 };
    PTensor t2 = {
        SData::getTypeIdentifier<CBasicType<double>>(),
        1,
        dim2,
        3,
        data2
    };

    struct CInternalReceiver : public IVisitor<const PTensor&> {
        int visit(const PTensor& r) {
            return sCtx.Error();
        }
    }receiver;

    STensorSolver::getSolver()->multiply(t1, t2, &receiver);
}

void testNN() {
    SNeuralNetwork n;
    SNeuralNetwork::getFactory()->createDense(1, n);

    double v = 0.5;
    int dimsize[] = { 1 };
    PTensor inputTensor;
    inputTensor.idType = SData::getTypeIdentifier<CBasicType<double>>();
    inputTensor.nData = 1;
    inputTensor.pData = &v;
    inputTensor.nDims = 1;
    inputTensor.pDimSizes = dimsize;
    struct LearnCtx : public SNeuralNetwork::ILearnCtx {
        int getOutputDeviation(const PTensor& outputTensor, PTensor& outputDeivation) {
            double y = 0.7*(*pV) - 0.3;
            *outputDeivation.pDoubleArray = 1/(1+exp(-y)) - outputTensor.pDoubleArray[0];
            return sCtx.Success();
        }
        double *pV;
    }ctx;
    ctx.pV = &v;

    for( int i=1; i<1000; i++) {
        v = rand() % 100 / 50.0;
        n->learn(inputTensor, &ctx, nullptr);
    }
}

void testIdx() {
    SData spTensor;
    SNeuralNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-images-idx3-ubytejUIsewocHHkkWlvPB_6G4z7q_ueSuEWErsJ29aLbxOY.gz", spTensor);
    //SNeuralNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_t10k-labels-idx1-ubyte965g-S4A7G3r0jpgiMMdvSNx7KP_oN7677JZkkIErsY.gz", spTensor);
    //SNeuralNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_train-images-idx3-ubyteRA_Kv3PMVG-iFHXoHqNwJlYF9WviEKQCTSyo8gNSNgk.gz", spTensor);
    //SNeuralNetwork::getFactory()->readIdxFile("D:\\Workspace\\tensorflow\\tensorflow_datas\\downloads\\extracted\\GZIP.cvdf-datasets_mnist_train-labels-idx1-ubyteNVJTSgpVi77WrtMrMMSVzKI9Vn7FLKyL4aBzDoAQJVw.gz", spTensor);

    const PTensor* pTensor = spTensor.getDataPtr<PTensor>();
    std::cout << pTensor->nData;
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
    testIdx();

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
