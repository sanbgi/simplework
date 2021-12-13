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
    SIMPLEWORK_FACTORY_REGISTER(CMyObject, SMyObject::getClassKey())




int main(int argc, char *argv[]){
    
    SVideoDevice sVideoDevice;
    while(SAvIn::getVideoDevice(sVideoDevice) == Error::ERRORTYPE_SUCCESS) {
        std::cout << "video device: " << sVideoDevice->getDeviceName() << "\n";
    }

    SAudioDevice sAudioDevice;
    while(SAvIn::getAudioDevice(sAudioDevice) == Error::ERRORTYPE_SUCCESS) {
        std::cout << "audio device: " << sAudioDevice->getDeviceName() << "\n";
    }


    //SAvIn avIn = SAvIn::openVideoFile("d:/tt.mkv");    
    //SAvIn avIn = SAvIn::openVideoDevice("vfwcap");
    SAvIn avIn = SAvIn::openVideoDevice("video=Integrated Camera");
    //SAvIn avIn = SAvIn::openAudioDevice("audio=麦克风阵列 (Realtek(R) Audio)");
    SAvOut avVideoOut = SAvOut::openWindow("Display Video", 640, 360);
    SAvOut avAudioOut = SAvOut::openSpeaker(nullptr, 44100, 2);
    int nframeVideo = 0;
    int nframeAudio = 0;
    int nframeUnknown = 0;
    int nframe = 0;
    SAvFrame frame;
    while(avIn->getFrame(frame) == Error::ERRORTYPE_SUCCESS) {
        switch(frame->getFrameType()){ 
        case SAvFrame::AVSTREAMTYPE_VIDEO:
            {
                avVideoOut->putFrame(frame);
                nframeVideo++;
            }
            break;

        case SAvFrame::AVSTREAMTYPE_AUDIO:
            {
                avAudioOut->putFrame(frame);
                nframeAudio++;
            }
            break;

        default:
            nframeUnknown++;
            break;
        }
        if( nframe++ % 10 == 0) {
            std::cout << "nframeVideo:" << nframeVideo << ", nframeAudio:" << nframeAudio << ", nframeUnknown:" << nframeUnknown << "\n";
        }
    }
    std::cout << "nframeVideo:" << nframeVideo << ", nframeAudio:" << nframeAudio << ", nframeUnknown:" << nframeUnknown << "\n";

    /*
    MyObject myObject = SObject::createObject<MyObject>();
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
