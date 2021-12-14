#include "av_sdl.h"
#include "CAvSampleType.h"

SDL_NAMESPACE_ENTER

EAvSampleType CAvSampleType::convert(SDL_AudioFormat eSampleFormat){
    switch(eSampleFormat) {
    case AUDIO_S16:
        return EAvSampleType::AvSampleType_Audio_S16;

    case AUDIO_U8:
        return EAvSampleType::AvSampleType_Audio_U8;
    }
    return EAvSampleType::AvSampleType_None;
}

SDL_AudioFormat CAvSampleType::toAudioFormat(EAvSampleType eAvType) {
    switch(eAvType) {
    case EAvSampleType::AvSampleType_Audio_S16:
        return AUDIO_S16;

    case EAvSampleType::AvSampleType_Audio_U8:
        return AUDIO_U8;
    }
    return 0;
}

EAvSampleType CAvSampleType::convert(SDL_PixelFormatEnum eSampleFormat) {
    switch(eSampleFormat) {
    case SDL_PIXELFORMAT_RGB24:
        return EAvSampleType::AvSampleType_Video_RGB;

    case SDL_PIXELFORMAT_RGBA32:
        return EAvSampleType::AvSampleType_Video_RGBA;
    }
    return EAvSampleType::AvSampleType_None;
}

SDL_PixelFormatEnum CAvSampleType::toPixelFormat(EAvSampleType eAvType) {
    switch(eAvType) {
    case EAvSampleType::AvSampleType_Video_RGB:
        return SDL_PIXELFORMAT_RGB24;

    case EAvSampleType::AvSampleType_Video_RGBA:
        return SDL_PIXELFORMAT_RGBA32;
    }
    return SDL_PIXELFORMAT_UNKNOWN;
}

SDL_NAMESPACE_LEAVE
