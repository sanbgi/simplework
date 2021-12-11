
#include "av.h"
#include <SDL2/SDL.h>

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

class CAvOut_SDLSpeaker : public CObject, public IAvOut{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int initSpeaker(const char* szName) {
        
        release();

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            return Error::ERRORTYPE_FAILURE;

        // B2. 打开音频设备并创建音频处理线程
        // B2.1 打开音频设备，获取SDL设备支持的音频参数actual_spec(期望的参数是wanted_spec，实际得到actual_spec)
        // 1) SDL提供两种使音频设备取得音频数据方法：
        //    a. push，SDL以特定的频率调用回调函数，在回调函数中取得音频数据
        //    b. pull，用户程序以特定的频率调用SDL_QueueAudio()，向音频设备提供数据。此种情况wanted_spec.callback=NULL
        // 2) 音频设备打开后播放静音，不启动回调，调用SDL_PauseAudio(0)后启动回调，开始正常播放音频
        SDL_AudioSpec wanted_spec, actual_spec;
        //wanted_spec.freq = p_codec_ctx->sample_rate;    // 采样率
        wanted_spec.format = AUDIO_S16SYS;              // S表带符号，16是采样深度，SYS表采用系统字节序
        //wanted_spec.channels = p_codec_ctx->channels;   // 声道数
        wanted_spec.silence = 0;                        // 静音值
        //wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;    // SDL声音缓冲区尺寸，单位是单声道采样点尺寸x通道数
        wanted_spec.callback = nullptr;                 // 回调函数，若为NULL，则应使用SDL_QueueAudio()机制
        //wanted_spec.userdata = p_codec_ctx;             // 提供给回调函数的参数
        if (SDL_OpenAudio(&wanted_spec, &actual_spec) < 0)
        {
            return Error::ERRORTYPE_FAILURE;
        }

        return Error::ERRORTYPE_SUCCESS;
    }

    int putFrame(const AvFrame& frame) {
        if(frame->getStreamingType() != AvStreaming::AVSTREAMTYPE_AUDIO ) {
            return Error::ERRORTYPE_FAILURE;
        }
        return Error::ERRORTYPE_SUCCESS;
    }

public:
    CAvOut_SDLSpeaker() {
    }
    ~CAvOut_SDLSpeaker() {
        release();
    }    

    void release() {
        SDL_Quit();
    }

private:

};
