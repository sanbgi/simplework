
#include <math.h>
#include <vector>
#include <iostream>
#include "CAvNetwork.h"

using namespace sw;
using namespace sw::av;
using namespace sw::math;
using namespace sw::nn;

static SCtx sCtx("CAvNetwork.Test");
void CAvNetwork::run() {

    SAvIn spIn = SAvIn::openAvFileReader("D://tt.mkv");
    int nStreamings = spIn->getStreamingSize();
    const PAvStreaming* pStreamings = spIn->getStreamingAt(0);
    SAvOut spFile = SAvOut::openAvFileWriter("D://tt2.mkv", nStreamings, pStreamings);
    SAvOut spWindow = SAvOut::openWindow("TestWindow", 720, 360);
    SAvOut spSpeaker;
    for(int i=0; i<nStreamings; i++) {
        if(pStreamings[i].frameMeta.sampleType == EAvSampleType::AvSampleType_Audio) {
           spSpeaker = SAvOut::openSpeaker(nullptr, pStreamings[i].frameMeta);
        }
    }
    
    int nFrames=0;
    SAvFrame spFrame;
    while(spIn->readFrame(spFrame) == sCtx.success()) {
        nFrames++;
        spFile->writeFrame(spFrame);
        const PAvFrame* pFrame = spFrame->getFramePtr();
        switch (pFrame->sampleMeta.sampleType)
        {
        case EAvSampleType::AvSampleType_Video:
            spWindow->writeFrame(spFrame);
            break;
        
        case EAvSampleType::AvSampleType_Audio:
            spSpeaker->writeFrame(spFrame);
            break;

        default:
            break;
        }
    }
    if(!spIn->isCompleted()) {
        sCtx.error("读取没有完成");
        return;
    }

    spFrame.release();
    if( spFile->writeFrame(spFrame) != sCtx.success() ) {
        sCtx.error("写入文件结尾失败");
    }
}
