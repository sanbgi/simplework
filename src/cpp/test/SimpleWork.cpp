#include <iostream>
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
    SIMPLEWORK_FACTORY_REGISTER(CMyObject, MyObject::getClassKey())


int main(int argc, char *argv[]){
    
    AvIn avIn = AvIn::openVideoFile("d:/tt.mkv");    
    //AvIn avIn = AvIn::openVideoCapture("vfwcap");
    AvOut avVideoOut = AvOut::openWindow("Display Video", 640, 360);
    AvOut avAudioOut = AvOut::openSpeaker(nullptr, 44000, 2);
    int nframeVideo = 0;
    int nframeAudio = 0;
    int nframeUnknown = 0;
    int nframe = 0;
    AvFrame frame;
    while(avIn->getFrame(frame) == Error::ERRORTYPE_SUCCESS) {
        switch(frame->getFrameType()){ 
        case AvFrame::AVSTREAMTYPE_VIDEO:
            {
                avVideoOut->putFrame(frame);
                nframeVideo++;
            }
            break;

        case AvFrame::AVSTREAMTYPE_AUDIO:
            {
                avAudioOut->putFrame(frame);
                nframeAudio++;
            }
            break;

        default:
            nframeUnknown++;
            break;
        }
        if( nframe++ % 100 == 0) {
            std::cout << "nframeVideo:" << nframeVideo << ", nframeAudio:" << nframeAudio << ", nframeUnknown:" << nframeUnknown << "\n";
        }
    }
    std::cout << "nframeVideo:" << nframeVideo << ", nframeAudio:" << nframeAudio << ", nframeUnknown:" << nframeUnknown << "\n";

    /*
    MyObject myObject = Object::createObject<MyObject>();
    if( myObject ) {
        myObject.sayHi();
    }else {
        std::cout << "??？ What happen?";
    }
    */

    return 0;
}


void say_hello(){


    /*
    TObject ppo;
    TObject ptr1, ptr0, ptrn(ptr1);
    Vector ppt;
    Vector ptr2, ptr3(ptr2);
    //char szV[] = "hi";
    //swcore::Vector ptrTensor(sizeof(szV)/sizeof(char), szV);
    //char* ptrChar = ptrTensor.getDataPtr<char>();
    //swcore::Vector tensor = swcore::Vector::createTensor<char>(1);
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
