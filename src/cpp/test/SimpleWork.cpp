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
    while(SAvIn::getVideoDevice(sVideoDevice) == SError::ERRORTYPE_SUCCESS) {
        std::cout << "video device: " << sVideoDevice->getDeviceName() << "\n";
    }

    SAudioDevice sAudioDevice;
    while(SAvIn::getAudioDevice(sAudioDevice) == SError::ERRORTYPE_SUCCESS) {
        std::cout << "audio device: " << sAudioDevice->getDeviceName() << "\n";
    }


    SAvIn avIn = SAvIn::openVideoFile("d:/tt.mkv");
    //SAvIn avIn = SAvIn::openVideoDevice("vfwcap");
    //SAvIn avIn = SAvIn::openVideoDevice("video=Integrated Camera");
    //SAvIn avIn = SAvIn::openAudioDevice("audio=麦克风阵列 (Realtek(R) Audio)");
    int iVideoId = -1;
    int iAudioId = -1;
    CAvSampleMeta videoMeta, audioMeta;
    SAvStreaming spStreaming;
    while(avIn->getStreaming(spStreaming) == SError::ERRORTYPE_SUCCESS) {
        std::cout << "streaming type: " << spStreaming->getStreamingType() << "\n";
        switch(spStreaming->getStreamingType()) {
        case EAvStreamingType::AvStreamingType_Video:
            if(iVideoId == -1) {
                iVideoId = spStreaming->getStreamingId();
                videoMeta = spStreaming->getSampleMeta();
                videoMeta.nVideoWidth = 640;
                videoMeta.nVideoHeight = 360;
                avIn->changeStreamingSampleMeta(iVideoId, videoMeta);
            }
            break;

        case EAvStreamingType::AvStreamingType_Audio:
            if(iAudioId == -1) {
                iAudioId = spStreaming->getStreamingId();
                audioMeta = spStreaming->getSampleMeta();
            }
            break;
        }
    }


    SAvOut avVideoOut = SAvOut::openWindow("Display Video", videoMeta);
    SAvOut avAudioOut = SAvOut::openSpeaker(nullptr, audioMeta);
    int nframeVideo = 0;
    int nframeAudio = 0;
    int nframeUnknown = 0;
    int nframe = 0;
    SAvFrame frame;
    int iStreamingId = 0;
    while(avIn->readFrame(iStreamingId, frame) == SError::ERRORTYPE_SUCCESS) {
        switch(frame->getStreamingType()){ 
        case EAvStreamingType::AvStreamingType_Video:
            {
                avVideoOut->writeFrame(frame);
                nframeVideo++;
            }
            break;

        case EAvStreamingType::AvStreamingType_Audio:
            {
                avAudioOut->writeFrame(frame);
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
