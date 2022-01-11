#include "av_ffmpeg.h"
#include "CAvSampleType.h"
#include <map>

FFMPEG_NAMESPACE_ENTER

/*
static std::map<AVSampleFormat, EAvSampleFormat> g_mapSampleFormatToAv = {
    { AV_SAMPLE_FMT_U8, AvSampleFormat_Audio_U8 },
    { AV_SAMPLE_FMT_S16, AvSampleFormat_Audio_S16 },
};

static std::map<AVPixelFormat, EAvSampleFormat> g_mapPixelFormatToAv = {
    { AV_PIX_FMT_RGB24, AvSampleFormat_Video_RGB },
    { AV_PIX_FMT_RGBA, AvSampleFormat_Video_RGBA },
};*/

EAvSampleFormat CAvSampleType::convert(AVSampleFormat eSampleFormat){
    switch (eSampleFormat)
    {
    case AV_SAMPLE_FMT_U8:
        return EAvSampleFormat::AvSampleFormat_Audio_U8;
    
    case AV_SAMPLE_FMT_S16:
        return EAvSampleFormat::AvSampleFormat_Audio_S16;
    }
    //return EAvSampleFormat::AvSampleFormat_None;
    return (EAvSampleFormat)eSampleFormat;
}

EAvSampleFormat CAvSampleType::convert(AVPixelFormat ePixelFormat){
    switch(ePixelFormat) {
    case AV_PIX_FMT_RGB24:
        return EAvSampleFormat::AvSampleFormat_Video_RGB;

    case AV_PIX_FMT_RGBA:
        return EAvSampleFormat::AvSampleFormat_Video_RGBA;
    }
    return (EAvSampleFormat)ePixelFormat;
}

AVSampleFormat CAvSampleType::toSampleFormat(EAvSampleFormat eAvType) {
    switch(eAvType) {
    case EAvSampleFormat::AvSampleFormat_Audio_U8:
        return AV_SAMPLE_FMT_U8;

    case EAvSampleFormat::AvSampleFormat_Audio_S16:
        return AV_SAMPLE_FMT_S16;
    }
    return (AVSampleFormat)eAvType;
}

AVPixelFormat CAvSampleType::toPixFormat(EAvSampleFormat eAvType) {
    switch(eAvType) {
    case EAvSampleFormat::AvSampleFormat_Video_RGB:
        return AV_PIX_FMT_RGB24;

    case EAvSampleFormat::AvSampleFormat_Video_RGBA:
        return AV_PIX_FMT_RGBA;

    /*
    case AV_PIX_FMT_YUVJ420P :
        return AV_PIX_FMT_YUV420P;

    case AV_PIX_FMT_YUVJ422P  :
        return AV_PIX_FMT_YUV422P;

    case AV_PIX_FMT_YUVJ444P   :
        return AV_PIX_FMT_YUV444P;

    case AV_PIX_FMT_YUVJ440P :
        return AV_PIX_FMT_YUV440P;
    */
    }

    return (AVPixelFormat)eAvType;
}

FFMPEG_NAMESPACE_LEAVE
