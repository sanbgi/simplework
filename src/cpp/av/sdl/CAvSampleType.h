#ifndef __SimpleWork_Av_sdl_CAvSampleType_h__
#define __SimpleWork_Av_sdl_CAvSampleType_h__

#include "av_sdl.h"

using namespace sw::av;

SDL_NAMESPACE_ENTER

class CAvSampleType {
public:
    static EAvSampleType convert(SDL_AudioFormat eSampleFormat);
    static SDL_AudioFormat toAudioFormat(EAvSampleType eAvType);
    
    static EAvSampleType convert(SDL_PixelFormatEnum eSampleFormat);
    static SDL_PixelFormatEnum toPixelFormat(EAvSampleType eAvType);

};

SDL_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_sdl_CAvSampleType_h__