#ifndef __SimpleWork_Av_sdl_CAvSampleFormat_h__
#define __SimpleWork_Av_sdl_CAvSampleFormat_h__

#include "av_sdl.h"

using namespace sw;

SDL_NAMESPACE_ENTER

class CAvSampleType {
public:
    static EAvSampleFormat convert(SDL_AudioFormat eSampleFormat);
    static SDL_AudioFormat toAudioFormat(EAvSampleFormat eAvType);
    
    static EAvSampleFormat convert(SDL_PixelFormatEnum eSampleFormat);
    static SDL_PixelFormatEnum toPixelFormat(EAvSampleFormat eAvType);

};

SDL_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_sdl_CAvSampleFormat_h__