
#include "av_ffmpeg.h"
#include "CAvOut.h"

FFMPEG_NAMESPACE_ENTER

int CAvOut::initAvFile(const char* szFileName, int nStreamings, SAvStreaming* pStreamings) {

    return 0;
}

int CAvOut::writeFrame(const SAvFrame& rFrame) {
    return SError::ERRORTYPE_FAILURE;
}

CAvOut::CAvOut() {
}

CAvOut::~CAvOut() {
    release();
}

void CAvOut::release() {
}


FFMPEG_NAMESPACE_LEAVE
