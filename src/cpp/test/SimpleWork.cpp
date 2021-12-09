#include <iostream>
#include "../inc/SimpleWork.h"

using namespace sw;
using namespace sw::av;

    SIMPLEWORK_INTERFACECLASS_ENTER(MyObject)

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
    SIMPLEWORK_FACTORY_REGISTER(CMyObject, "TestSimpleWork.MyObject")


int main(int argc, char *argv[]){
    
    AvIn avIn = Object::createObject("sw.av.AvIn");
    avIn->init("d:/tt.mkv");
    int nframeVideo = 0;
    int nframeAudio = 0;
    int nframeUnknown = 0;
    AvFrame frame;
    while(avIn->getFrame(frame) == Error::ERRORTYPE_SUCCESS) {
        switch(frame->getFrameType()){ 
        case AvFrame::AVFRAMETYPE_VIDEO:
            nframeVideo++;
            break;

        case AvFrame::AVFRAMETYPE_AUDIO:
            nframeAudio++;
            break;

        default:
            nframeUnknown++;
            break;
        }
        if( nframeVideo % 100 == 0) {
            std::cout << "nframeVideo:" << nframeVideo << ", nframeAudio:" << nframeAudio << ", nframeUnknown:" << nframeUnknown << "\n";
        }
    }
    std::cout << "nframeVideo:" << nframeVideo << ", nframeAudio:" << nframeAudio << ", nframeUnknown:" << nframeUnknown << "\n";

    Object tensor = Object::createObject("sw.math.Tensor");
    Factory factory = Object::createFactory("sw.math.Tensor");
    tensor = factory;
    if( tensor ) {
        std::cout << "Great\n";
    }

    MyObject myObject = Object::createObject("TestSimpleWork.MyObject");
    if( myObject ) {
        myObject.sayHi();
    }else {
        std::cout << "??？ What happen?";
    }

    return 0;
}


void say_hello(){


    /*
    TObject ppo;
    TObject ptr1, ptr0, ptrn(ptr1);
    Tensor ppt;
    Tensor ptr2, ptr3(ptr2);
    //char szV[] = "hi";
    //swcore::Tensor ptrTensor(sizeof(szV)/sizeof(char), szV);
    //char* ptrChar = ptrTensor.getDataPtr<char>();
    //swcore::Tensor tensor = swcore::Tensor::createTensor<char>(1);
    IFactory* pFactory = nullptr;
    ITensorPtr ppO;
    ppO = pFactory;
    //ptr2 = pFactory;
    ptr3 = ptr2;
    ptr2 = ptr1;
    ptr1 = ptr2;
    ptr3 = ptr2 = ptr1 = ptr0;
    ptr3 = ptr2.getPtr();
    */
}
