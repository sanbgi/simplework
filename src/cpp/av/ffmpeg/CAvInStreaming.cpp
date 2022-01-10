#include "av_ffmpeg.h"
#include "CAvInStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvInStreaming");

CAvInStreaming::CAvInStreaming() {
    m_pAvStream = nullptr;
    m_isCompleted = false;
}

CAvInStreaming::~CAvInStreaming() {
    release();
}

void CAvInStreaming::release() {
}

int CAvInStreaming::init(AVStream* pAvStream, int iStreamingIndex) {
    AVCodecParameters* pCodecParameter = pAvStream->codecpar;
    if(pCodecParameter == nullptr) {
        return sCtx.error();
    }

    CTaker<AVCodecContext*> pCodecCtx(  avcodec_alloc_context3(nullptr), 
                                        [](AVCodecContext* pCtx){
                                            avcodec_free_context(&pCtx);
                                        });
    if( avcodec_parameters_to_context(pCodecCtx, pCodecParameter) < 0 ) {
        return sCtx.error();
    }

    AVCodec* pCodec=avcodec_find_decoder(pCodecParameter->codec_id);
    if(pCodec==NULL){
        printf("Codec not found.\n");
        return sCtx.error();
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
        printf("Could not open codec.\n");
        return sCtx.error();
    }

    PAvStreaming& avStream = m_avStreaming;
    avStream.timeDuration = pAvStream->duration;
    avStream.streamingId = pAvStream->index;
    avStream.timeRate = pAvStream->time_base.den/pAvStream->time_base.num;
    AVCodecParameters* pCodecContext = pAvStream->codecpar;
    switch(pCodecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            {
                avStream.frameMeta.sampleType = EAvSampleType::AvSampleType_Audio;
                avStream.frameMeta.sampleFormat = CAvSampleType::convert((AVSampleFormat)pCodecContext->format);
                avStream.frameMeta.audioChannels = pCodecContext->channels;
                avStream.frameMeta.audioRate = pCodecContext->sample_rate;
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            {
                avStream.frameMeta.sampleType = EAvSampleType::AvSampleType_Video;
                avStream.frameMeta.sampleFormat = CAvSampleType::convert((AVPixelFormat)pCodecContext->format);
                avStream.frameMeta.videoWidth = pCodecContext->width;
                avStream.frameMeta.videoHeight = pCodecContext->height;
            }
            break;

        default:
            return sCtx.error("当前的流类型还未被支持");
    }

    m_spCodecCtx.take(pCodecCtx);
    m_pAvStream = pAvStream;
    return sCtx.success();
}

PAvStreaming& CAvInStreaming::getPAvStreaming() {
    return m_avStreaming;
}

FFMPEG_NAMESPACE_LEAVE