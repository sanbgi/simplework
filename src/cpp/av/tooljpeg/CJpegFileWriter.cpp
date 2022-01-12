#include "../av.h"
#include "CJpegFileWriter.h"
#include "toojpeg.h"
#include <iostream>
#include <fstream>

using namespace sw;
using namespace std;

static SCtx sCtx("CJpegFileWriter");

ofstream* s_ps = nullptr;
void writeByte(unsigned char c) {
    if(s_ps) {
        s_ps->write((const char*)&c, 1);
    }
}

int CJpegFileWriter::saveToFile(const char* szFileName, const SAvFrame& spFrame) {

    const PAvFrame* pFrame = spFrame->getFramePtr(); 

    PAvSample targetSample;
    targetSample.sampleType = EAvSampleType::AvSampleType_Video;
    targetSample.sampleFormat = EAvSampleFormat::AvSampleFormat_Video_RGB;
    targetSample.videoHeight = pFrame->nHeight;
    targetSample.videoWidth = pFrame->nWidth;

    SAvNetwork spConverter;
    if( SAvFactory::getAvFactory()->openAvFrameConverter(targetSample, spConverter) != sCtx.success() ) {
        return sCtx.error("创建格式转化器失败");
    }

    SAvFrame spOut;
    if( spConverter->pipeIn(spFrame, spOut) != sCtx.success() ) {
        return sCtx.error("图片格式转化失败");
    }

    pFrame = spOut->getFramePtr();

    ofstream file;
    file.open(szFileName, ios_base::binary);
    if( !file.is_open() ) {
        return sCtx.error(string(string("写图片文件失败，文件名: ") + szFileName).c_str());
    }

    s_ps = &file;
    bool retcode = TooJpeg::writeJpeg(writeByte, pFrame->ppPlanes[0], pFrame->nWidth, pFrame->nHeight, true);
    s_ps = nullptr;

    if(!retcode) {
        return sCtx.error("转化图片失败");
    }
    file.flush();
    file.close();
    return sCtx.success();
}