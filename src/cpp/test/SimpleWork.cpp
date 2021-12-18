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



int testPlayFile() {
    //SAvIn avIn = SAvIn::openVideoFile("d:/tt.mkv");
    //SAvIn avIn = SAvIn::openVideoDevice("vfwcap");
    SAvIn avIn = SAvIn::openVideoDevice("video=Integrated Camera");
    //SAvIn avIn = SAvIn::openAudioDevice("audio=麦克风阵列 (Realtek(R) Audio)");
    std::vector<PAvStreaming> arrStreamings;
    CPVisitor<std::vector<PAvStreaming>*, const PAvStreaming*> visitor(&arrStreamings, [](std::vector<PAvStreaming>* pArr, const PAvStreaming* pStreaming)->int{
        pArr->push_back(*pStreaming);
        return SError::ERRORTYPE_SUCCESS;
    });
    if( avIn->visitStreamings(&visitor) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }

    int iVideoId = -1;
    int iAudioId = -1;
    PAvSample videoMeta, audioMeta;
    for(int i=0; i<arrStreamings.size(); i++) {
        PAvStreaming streaming = arrStreamings[i];
        switch(streaming.frameMeta.sampleType) {
        case EAvSampleType::AvSampleType_Video:
            if(iVideoId == -1) {
                iVideoId = streaming.streamingId;
                videoMeta = streaming.frameMeta;
            }
            break;

        case EAvSampleType::AvSampleType_Audio:
            if(iAudioId == -1) {
                iAudioId = streaming.streamingId;
                audioMeta = streaming.frameMeta;
            }
            break;
        }
    }

    videoMeta.videoWidth = 640;
    videoMeta.videoHeight = 360;
    SAvOut avAudioOut = SAvOut::openSpeaker(nullptr, audioMeta);
    struct CCtx {
        SAvOut video;
        SAvOut audio;
    };
    CCtx outCtx;
    //outCtx.video = SAvOut::openAvFile("D:\\cap.mkv", arrStreamings.size(), arrStreamings.data());
    outCtx.video = SAvOut::openWindow("Display Video", videoMeta);
    outCtx.audio = avAudioOut;
    int nframeVideo = 0;
    while(avIn->readFrame(CPVisitor<CCtx, const PAvFrame*>(outCtx, [](CCtx ctx, const PAvFrame* pFrame) -> int{
        switch(pFrame->sampleMeta.sampleType) {
        case EAvSampleType::AvSampleType_Audio:
            std::cout << "audioVideo timestamp:" << pFrame->timeStamp << "\n";
            return ctx.audio->writeFrame(pFrame);

        case  EAvSampleType::AvSampleType_Video:
            std::cout << "frameVideo timestamp:" << pFrame->timeStamp << "\n";
            return ctx.video->writeFrame(pFrame);
        }
        return SError::ERRORTYPE_FAILURE;
    })) == SError::ERRORTYPE_SUCCESS ) {
        nframeVideo++;
        if(nframeVideo > 500) {
            break;
        }
    }
    return outCtx.video->writeFrame(nullptr);
}

int testWriteFile() {

    SAvIn avIn = SAvIn::openVideoFile("d:/tt.mkv");

    std::vector<PAvStreaming> arrStreamings;
    CPVisitor<std::vector<PAvStreaming>*, const PAvStreaming*> visitor(&arrStreamings, [](std::vector<PAvStreaming>* pArr, const PAvStreaming* pStreaming)->int{
        pArr->push_back(*pStreaming);
        return SError::ERRORTYPE_SUCCESS;
    });
    if( avIn->visitStreamings(&visitor) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }

    int nframe = 0;
    SAvOut avOut = SAvOut::openAvFile("d://tt2.mkv", arrStreamings.size(), arrStreamings.data() );
    while(avIn->readFrame(CPVisitor<SAvOut, const PAvFrame*>(avOut, [](SAvOut avOut, const PAvFrame* pFrame) -> int{
        if(pFrame == nullptr) {
            //不再继续读取
            avOut->writeFrame(pFrame);
            return SError::ERRORTYPE_FAILURE;
        }
        std::cout << "timestamps:  " << pFrame->timeStamp << "\n";
        return avOut->writeFrame(pFrame);
    })) == SError::ERRORTYPE_SUCCESS ) {
        nframe++;
    }
    avOut->writeFrame(nullptr);
    return SError::ERRORTYPE_SUCCESS;
}


void fun(const CData& data) {
     if( data.hasData<int>() ) {
        std::cout << "i get the integer." << data.getData<int>();
     }
     if(data.hasData<double>() ) {
         std::cout << "i get the double." << data.getData<double>();
     }
 }

int main(int argc, char *argv[]){

    //testWriteFile();
    testPlayFile();

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
