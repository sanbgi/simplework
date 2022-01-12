#ifndef __SimpleWork_av_ffmpeg_h__
#define __SimpleWork_av_ffmpeg_h__

#define FFMPEG_NAMESPACE_ENTER namespace ffmpeg {
#define FFMPEG_NAMESPACE_LEAVE }

#include <string>
#include <vector>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>
    #include <libswresample/swresample.h>
    #include <libavutil/imgutils.h>
}
#include "../Av.h"

using namespace sw;
using namespace sw;
using namespace sw;

#endif//__SimpleWork_av_ffmpeg_h__