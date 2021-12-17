#ifndef __SimpleWork_Av_CAvSampleFormat_h__
#define __SimpleWork_Av_CAvSampleFormat_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvSampleType {
public:
    static EAvSampleFormat convert(AVSampleFormat eSampleFormat);
    static EAvSampleFormat convert(AVPixelFormat ePixelFormat);
    static AVSampleFormat toSampleFormat(EAvSampleFormat eAvType);
    static AVPixelFormat toPixFormat(EAvSampleFormat eAvType);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvSampleFormat_h__