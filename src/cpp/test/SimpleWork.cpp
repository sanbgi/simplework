#include <iostream>
#include <vector>

#include "../inc/SimpleWork.h"

using namespace sw;
using namespace sw::av;
using namespace sw::math;

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
    testPipe();

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
