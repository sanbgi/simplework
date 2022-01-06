#include "av_ffmpeg.h"
#include "CAvStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvStreaming");

CAvStreaming::CAvStreaming() {
    m_pAvStream = nullptr;
    m_iStreamingIndex = -1;
}

CAvStreaming::~CAvStreaming() {
    release();
}

void CAvStreaming::release() {
}

EAvSampleType CAvStreaming::getSampleType() {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return EAvSampleType::AvSampleType_Video;
        
        case AVMEDIA_TYPE_AUDIO:
            return EAvSampleType::AvSampleType_Audio;
        }
    }
    return EAvSampleType::AvSampleType_None;
}

int CAvStreaming::getStreamingId() {
    return m_iStreamingIndex;
}

int CAvStreaming::getTimeRate() {
    AVRational r = m_pAvStream->time_base;
    return r.den/r.num;
}

int CAvStreaming::getSampleRate() {
    return m_spCodecCtx->sample_rate;
}

long CAvStreaming::getDuration() {
    return m_pAvStream->duration;
}

EAvSampleFormat CAvStreaming::getSampleFormat() {
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            EAvSampleFormat eType = CAvSampleType::convert(m_spCodecCtx->pix_fmt);
            if(eType == EAvSampleFormat::AvSampleFormat_None) {
                eType = EAvSampleFormat::AvSampleFormat_Video_RGBA;
            }
            return eType;
        }
        break;

    case AVMEDIA_TYPE_AUDIO:
        {
            EAvSampleFormat eType = CAvSampleType::convert(m_spCodecCtx->sample_fmt);
            if(eType == EAvSampleFormat::AvSampleFormat_None) {
                eType = EAvSampleFormat::AvSampleFormat_Audio_S16;
            }
            return eType;
        }
        break;
    }
    return EAvSampleFormat::AvSampleFormat_None;
}

const PAvSample& CAvStreaming::getSampleMeta() {
    return m_sampleMeta;
}

int CAvStreaming::setSampleMeta(const PAvSample& sampleMeta) {
    m_sampleMeta = sampleMeta;
    return sCtx.success();
}

int CAvStreaming::init(AVStream* pAvStream, int iStreamingIndex) {
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

    m_spCodecCtx.take(pCodecCtx);
    m_iStreamingIndex = iStreamingIndex;
    m_pAvStream = pAvStream;
    m_sampleMeta.sampleFormat = getSampleFormat();
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            m_sampleMeta.videoWidth = m_spCodecCtx->width;
            m_sampleMeta.videoHeight = m_spCodecCtx->height;
            m_sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        {
            m_sampleMeta.audioRate = m_spCodecCtx->sample_rate;
            m_sampleMeta.audioChannels = m_spCodecCtx->channels;
            m_sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio;
        }
        break;
    }

    if( SAvFactory::getAvFactory()->openAvFrameConverter(m_sampleMeta, m_spConverter) != sCtx.success()) {
        return sCtx.error();
    }

    return sCtx.success();
}


int CAvStreaming::receiveFrame(PAvFrame::FVisitor receiver, AVFrame* pAvFrame) {
    PAvFrame avFrame;
    avFrame.sampleMeta = m_sampleMeta;

    unsigned char* ppData[AV_NUM_DATA_POINTERS+1];
    int pLineSize[AV_NUM_DATA_POINTERS+1];
    avFrame.ppPlanes = ppData;
    avFrame.pPlaneLineSizes = pLineSize;

    //
    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据，这里面要注意，是否存在
    //  planar audio，并且通道数超过8？如果存在这种情况，则这里的数据是存在丢失的
    //
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pAvFrame->data[i]; i++ ) {
        nPlanes = i+1;
        ppData[i] = pAvFrame->data[i];
        pLineSize[i] = m_sampleMeta.sampleType == EAvSampleType::AvSampleType_Audio ? 
                            pAvFrame->linesize[0] :
                            pAvFrame->linesize[i];
    }
    ppData[nPlanes] = nullptr;
    pLineSize[nPlanes] = 0;

    avFrame.nPlanes = nPlanes;
    avFrame.ppPlanes = ppData;
    avFrame.pPlaneLineSizes = pLineSize;
    avFrame.streamingId = getStreamingId();
    avFrame.timeRate = getTimeRate();
    avFrame.timeStamp = pAvFrame->pts;
    switch(m_sampleMeta.sampleType) {
        case EAvSampleType::AvSampleType_Audio:
        {
            avFrame.nWidth = pAvFrame->nb_samples;
            avFrame.nHeight = 1;
        }
        break;

        case EAvSampleType::AvSampleType_Video:
        {
            avFrame.nWidth = pAvFrame->width;
            avFrame.nHeight = pAvFrame->height;
        }
        break;

        default:
            return sCtx.error();
    }

    struct CInternalReceiver : IVisitor<const PData&> {
        int visit(const PData& rData) {
            return receiver->visit( CData<PAvFrame>(rData) );
        } 
        PAvFrame::FVisitor receiver;
    }thisReceiver;
    thisReceiver.receiver = receiver;

    return m_spConverter->pushData(CData<PAvFrame>(avFrame), &thisReceiver);
}

FFMPEG_NAMESPACE_LEAVE