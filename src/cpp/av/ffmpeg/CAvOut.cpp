
#include "av_ffmpeg.h"
#include "CAvOut.h"

FFMPEG_NAMESPACE_ENTER

int CAvOut::pubFrame(SAvFrame& rFrame) {
    return Error::ERRORTYPE_FAILURE;
}

CAvOut::CAvOut() {
}

CAvOut::~CAvOut() {
    release();
}

void CAvOut::release() {
}


FFMPEG_NAMESPACE_LEAVE
