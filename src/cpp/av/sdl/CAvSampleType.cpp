#include "av_sdl.h"
#include "CAvSampleType.h"

SDL_NAMESPACE_ENTER

EAvSampleFormat CAvSampleType::convert(SDL_AudioFormat eSampleFormat){
    switch(eSampleFormat) {
    case AUDIO_S16:
        return EAvSampleFormat::AvSampleFormat_Audio_S16;

    case AUDIO_U8:
        return EAvSampleFormat::AvSampleFormat_Audio_U8;
    }
    return EAvSampleFormat::AvSampleFormat_None;
}

SDL_AudioFormat CAvSampleType::toAudioFormat(EAvSampleFormat eAvType) {
    switch(eAvType) {
    case EAvSampleFormat::AvSampleFormat_Audio_S16:
        return AUDIO_S16;

    case EAvSampleFormat::AvSampleFormat_Audio_U8:
        return AUDIO_U8;
    }
    return 0;
}

EAvSampleFormat CAvSampleType::convert(SDL_PixelFormatEnum eSampleFormat) {
    switch(eSampleFormat) {
    case SDL_PIXELFORMAT_RGB24:
        return EAvSampleFormat::AvSampleFormat_Video_RGB;

    case SDL_PIXELFORMAT_RGBA32:
        return EAvSampleFormat::AvSampleFormat_Video_RGBA;
    }
    return EAvSampleFormat::AvSampleFormat_None;
}

SDL_PixelFormatEnum CAvSampleType::toPixelFormat(EAvSampleFormat eAvType) {
    switch(eAvType) {
    case EAvSampleFormat::AvSampleFormat_Video_RGB:
        return SDL_PIXELFORMAT_RGB24;

    case EAvSampleFormat::AvSampleFormat_Video_RGBA:
        return SDL_PIXELFORMAT_RGBA32;
    }
    return SDL_PIXELFORMAT_UNKNOWN;
}

SDL_NAMESPACE_LEAVE
