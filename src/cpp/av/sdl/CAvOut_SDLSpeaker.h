
#ifndef __SimpleWork_av_sdl_CAvOut_SDLSpeaker_h__
#define __SimpleWork_av_sdl_CAvOut_SDLSpeaker_h__

#include "av_sdl.h"
#include "CAvSampleType.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SDL_NAMESPACE_ENTER

class CAvOut_SDLSpeaker : public CObject, public IAvOut, IVisitor<const PAvFrame*>{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int initSpeaker(const char* szName, PAvSample& sampleMeta) {
        
        release();

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            return SError::ERRORTYPE_FAILURE;

        int nTargetRate = sampleMeta.audioRate;
        int nTargetChannels = sampleMeta.audioChannels;
        SDL_AudioFormat eTargetFormat = CAvSampleType::toAudioFormat(sampleMeta.sampleType);

        // B2. 打开音频设备并创建音频处理线程
        // B2.1 打开音频设备，获取SDL设备支持的音频参数actual_spec(期望的参数是wanted_spec，实际得到actual_spec)
        // 1) SDL提供两种使音频设备取得音频数据方法：
        //    a. push，SDL以特定的频率调用回调函数，在回调函数中取得音频数据
        //    b. pull，用户程序以特定的频率调用SDL_QueueAudio()，向音频设备提供数据。此种情况wanted_spec.callback=NULL
        // 2) 音频设备打开后播放静音，不启动回调，调用SDL_PauseAudio(0)后启动回调，开始正常播放音频
        SDL_AudioSpec wanted_spec;
        wanted_spec.freq = nTargetRate;          // 采样率
        wanted_spec.format = eTargetFormat;      // S表带符号，16是采样深度，SYS表采用系统字节序
        wanted_spec.channels = nTargetChannels;  // 声道数
        wanted_spec.silence = 0;                 // 静音值
        wanted_spec.samples = 1024;              // SDL声音缓冲区尺寸，单位是单声道采样点尺寸x通道数
        wanted_spec.callback = nullptr;          // 回调函数，若为NULL，则应使用SDL_QueueAudio()机制
        //wanted_spec.userdata = p_codec_ctx;    // 提供给回调函数的参数
        m_iDeviceID = SDL_OpenAudioDevice(szName, false, &wanted_spec, &m_specAudio, 0);
        if(m_iDeviceID == 0)
        {
            return SError::ERRORTYPE_FAILURE;
        }

        PAvSample specMeta;
        specMeta.audioChannels = m_specAudio.channels;
        specMeta.audioRate = m_specAudio.freq;
        specMeta.sampleType = CAvSampleType::convert(m_specAudio.format);
        if( SAvFilter::createFilter(sampleMeta, m_spFilter) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }

        SDL_PauseAudioDevice(m_iDeviceID, 0);
        return SError::ERRORTYPE_SUCCESS;
    }


    int writeFrame(const SAvFrame& frame) {
        STensor tensor = frame->getData();
        if(tensor) {
            int ret = tensor->getDataSize();
            if( tensor ) {
                ret = SDL_QueueAudio(m_iDeviceID, tensor->getDataPtr<unsigned char>(), tensor->getDataSize());
            }
        }
        return SError::ERRORTYPE_SUCCESS;
    }

    int writeFrame(const PAvFrame* pFrame) {
        if(pFrame == nullptr) {
            return close();
        }

        return m_spFilter->putFrame(pFrame, this);
    }

    int visit(const PAvFrame* pFrame) {
        if(pFrame) {
            int ret = SDL_QueueAudio(
                            m_iDeviceID, pFrame->planeDatas[0], 
                            pFrame->planeLineSizes[0]);
        }
        return SError::ERRORTYPE_SUCCESS;
    }

    int close() {
        release();
        return SError::ERRORTYPE_SUCCESS;
    }

public:
    CAvOut_SDLSpeaker() {
        m_iDeviceID = 0;
    }
    ~CAvOut_SDLSpeaker() {
        release();
    }    

    void release() {
        if( m_iDeviceID ) {
            //TODO close device
            m_iDeviceID = 0;
        }
    }

private:
    SDL_AudioDeviceID m_iDeviceID;
    SDL_AudioSpec m_specAudio;
    SAvFilter m_spFilter;
};

SDL_NAMESPACE_LEAVE
#endif//__SimpleWork_av_sdl_CAvOut_SDLSpeaker_h__