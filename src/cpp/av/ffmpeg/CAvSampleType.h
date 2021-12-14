#ifndef __SimpleWork_Av_CAvSampleType_h__
#define __SimpleWork_Av_CAvSampleType_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvSampleType {
public:
    static EAvSampleType convert(AVSampleFormat eSampleFormat);
    static EAvSampleType convert(AVPixelFormat ePixelFormat);
    static AVSampleFormat toSampleFormat(EAvSampleType eAvType);
    static AVPixelFormat toPixFormat(EAvSampleType eAvType);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvSampleType_h__