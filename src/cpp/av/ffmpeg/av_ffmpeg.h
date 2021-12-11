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
template<typename Q> class CFFMpegPointer {
public:
    typedef void (*FUN)(Q**);
    CFFMpegPointer(Q* pQ, FUN fun) {
        m_ptr = pQ;
        m_fun = fun;
    }
    ~CFFMpegPointer() {
        if(m_ptr) {
            (*m_fun)(&m_ptr);
        }
    }
    Q* operator->() {
        return m_ptr;
    }
    operator Q*() {
        return m_ptr;
    }
    Q* detach() {
        Q* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

private:
    Q* m_ptr;
    FUN m_fun;
};
#include "../Av.h"

using namespace sw::core;
using namespace sw::math;
using namespace sw::av;

#endif//__SimpleWork_av_ffmpeg_h__