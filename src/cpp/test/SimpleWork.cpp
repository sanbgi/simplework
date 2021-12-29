#include <iostream>
#include <vector>

#include "../inc/SimpleWork.h"
#include <math.h>

using namespace sw;
using namespace sw::av;
using namespace sw::math;
using namespace sw::nn;

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
    SIMPLEWORK_PDATAKEY("FAA")

    int i;
};

struct FBB {
    SIMPLEWORK_PDATAKEY(FAA::__getClassKey())

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
    if( SAvFactory::getAvFactory()->openAvFileReader("d:/tt.mkv", avIn) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }

    SPipe avWindows;
    if( SAvFactory::getAvFactory()->openWindow("DisplayAv", 640, 360, avWindows) != SError::ERRORTYPE_SUCCESS ){
        return SError::ERRORTYPE_FAILURE;
    } 

    SPipe avSpeaker;
    if( SAvFactory::getAvFactory()->openSpeaker(nullptr, avSpeaker) ) {
        return SError::ERRORTYPE_FAILURE;
    }

    SPipe avFileWriter;
    if( SAvFactory::getAvFactory()->openAvFileWriter("d:/tt2.mkv", avFileWriter) ) {
        return SError::ERRORTYPE_FAILURE;
    }

    SPipe pipes[3] = { avWindows, avSpeaker, avFileWriter };
    SPipe avOut;
    if( SPipe::getFactory()->createParallelPipe(3, pipes, avOut) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }

    SPipe inPipe[2] = {avIn, avOut};
    SPipe av;
    if( SPipe::getFactory()->createSequencePipe(2, inPipe, av) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }

    int nFrames=0;
    while(av->pushData(CBasicData<int>(10), nullptr) == SError::ERRORTYPE_SUCCESS) {
        std::cout << "frames: " << ++nFrames << "processed \n";
    }

    return SError::ERRORTYPE_SUCCESS;
}

void testTensor() {

    int dim1[] = {2, 3};
    double data1[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    PTensor t1 = {
        SData::getBasicTypeIdentifier<double>(),
        2,
        dim1,
        6,
        data1
    };

    int dim2[] = {3, 1};
    double data2[] = { 6.0, 7.0, 8.0, 9.0, 10.0, 11.0 };
    PTensor t2 = {
        SData::getBasicTypeIdentifier<double>(),
        1,
        dim2,
        3,
        data2
    };

    struct CInternalReceiver : public IVisitor<const PTensor&> {
        int visit(const PTensor& r) {
            return SError::ERRORTYPE_FAILURE;
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
    inputTensor.idType = SData::getBasicTypeIdentifier<double>();
    inputTensor.nData = 1;
    inputTensor.pData = &v;
    inputTensor.nDims = 1;
    inputTensor.pDimSizes = dimsize;
    struct LearnCtx : public SNeuralNetwork::ILearnCtx {
        int getOutputDelta(const PTensor& outputTensor, IVisitor<const PTensor&>* pTargetReceiver) {

            double y = 0.7*(*pV) - 0.3;
            double v = 1/(1+exp(-y)) - outputTensor.pDoubleArray[0];
            int dimsize[] = { 1 };
            PTensor targetTensor;
            targetTensor.idType = SData::getBasicTypeIdentifier<double>();
            targetTensor.nData = 1;
            targetTensor.pData = &v;
            targetTensor.nDims = 1;
            targetTensor.pDimSizes = dimsize;
            return pTargetReceiver->visit(targetTensor);
        }

        //
        // 返回期望输入量对目标的偏差量
        //
        int pubInputDelta(const PTensor& inputDelta){
            return SError::ERRORTYPE_SUCCESS;
        }

        double *pV;
    }ctx;
    ctx.pV = &v;

    for( int i=1; i<1000; i++) {
        v = rand() % 100 / 50.0;
        n->learn(inputTensor, 0, &ctx);
    }
}

int main(int argc, char *argv[]){

    //FAA a = { 10 };
    //FBB b = { 20 };
    //fun(CData<FAA>(a));
    //fun(CData<FBB>(b));
    //unsigned int la = SData::getBasicTypeIdentifier<int>();
    //unsigned long lb = SData::getStructTypeIdentifier("aaaaaaaaaaaaaaaafadfasdf");
    //unsigned long lc = SData::getStructTypeIdentifier("ffffffffffffffffasdfasdfasdfasdfa");
    //unsigned long ld = SData::getStructTypeIdentifier("ffffffffffffffffffasdfasdfadsfadsfasdf");
    //testWriteFile();
    //testPipe();
    //testTensor();
    testNN();

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
