#include "av_ffmpeg.h"
#include "CAvSampleType.h"
#include <map>

FFMPEG_NAMESPACE_ENTER

/*
static std::map<AVSampleFormat, EAvSampleType> g_mapSampleFormatToAv = {
    { AV_SAMPLE_FMT_U8, AvSampleType_Audio_U8 },
    { AV_SAMPLE_FMT_S16, AvSampleType_Audio_S16 },
};

static std::map<AVPixelFormat, EAvSampleType> g_mapPixelFormatToAv = {
    { AV_PIX_FMT_RGB24, AvSampleType_Video_RGB },
    { AV_PIX_FMT_RGBA, AvSampleType_Video_RGBA },
};*/

EAvSampleType CAvSampleType::convert(AVSampleFormat eSampleFormat){
    switch (eSampleFormat)
    {
    case AV_SAMPLE_FMT_U8:
        return SAvSample::AvSampleType_Audio_U8;
    
    case AV_SAMPLE_FMT_S16:
        return SAvSample::AvSampleType_Audio_S16;
    }
    return EAvSampleType::AvSampleType_None;
}

EAvSampleType CAvSampleType::convert(AVPixelFormat ePixelFormat){
    switch(ePixelFormat) {
    case AV_PIX_FMT_RGB24:
        return SAvSample::AvSampleType_Video_RGB;

    case AV_PIX_FMT_RGBA:
        return SAvSample::AvSampleType_Video_RGBA;
    }
    return EAvSampleType::AvSampleType_None;
}

AVSampleFormat CAvSampleType::toSampleFormat(EAvSampleType eAvType) {
    switch(eAvType) {
    case SAvSample::AvSampleType_Audio_U8:
        return AV_SAMPLE_FMT_U8;

    case SAvSample::AvSampleType_Audio_S16:
        return AV_SAMPLE_FMT_S16;
    }
    return AVSampleFormat::AV_SAMPLE_FMT_NONE;
}

AVPixelFormat CAvSampleType::toPixFormat(EAvSampleType eAvType) {
    switch(eAvType) {
    case SAvSample::AvSampleType_Video_RGB:
        return AV_PIX_FMT_RGB24;

    case SAvSample::AvSampleType_Video_RGBA:
        return AV_PIX_FMT_RGBA;
    }
    return AVPixelFormat::AV_PIX_FMT_NONE;
}

FFMPEG_NAMESPACE_LEAVE
