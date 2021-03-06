#include "av_ffmpeg.h"
#include "CAvOutStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER
static SCtx sCtx("CAvOutStreaming");
CAvOutStreaming::CAvOutStreaming() {
    m_pAvStream = nullptr;
    m_eStreamingType = EAvSampleType::AvSampleType_None;
    m_iStreamingId = -1;
    //m_pCodec = nullptr;
    m_nWriteNumber = 0;
    m_nDuration = 0;
}

CAvOutStreaming::~CAvOutStreaming() {
    release();
}

void CAvOutStreaming::release() {
}

EAvSampleType CAvOutStreaming::getSampleType() {
    return m_eStreamingType;
}

int CAvOutStreaming::getStreamingId() {
    return m_iStreamingId;
}

int CAvOutStreaming::getTimeRate() {
    return m_nTimeRate;
}

long  CAvOutStreaming::getDuration() {
    return m_nDuration;
}

const PAvSample& CAvOutStreaming::getSampleMeta() {
    return m_sampleMeta;
}

int CAvOutStreaming::init(AVFormatContext* pFormatContext, const PAvStreaming* pSrc) {
    switch(pSrc->frameMeta.sampleType) {
    case EAvSampleType::AvSampleType_Video:
        return initVideo(pFormatContext, pSrc);

    case EAvSampleType::AvSampleType_Audio:
        return initAudio(pFormatContext, pSrc);
    }
    return sCtx.error();
}

int CAvOutStreaming::initVideo(AVFormatContext* pFormatContext, const PAvStreaming* pSrc){
    
    m_nTimeRate = 24;

    const AVOutputFormat* pOutputFormat = pFormatContext->oformat;
    if(pOutputFormat == nullptr || pOutputFormat->video_codec == AV_CODEC_ID_NONE) {
        return sCtx.error();
    }

    // 获取Codec
    AVCodec* pCodec = avcodec_find_encoder(pOutputFormat->video_codec);
    if(pCodec == nullptr) {
        return sCtx.error();
    }

    // 创建流
    AVStream* pAvStream = avformat_new_stream(pFormatContext, nullptr);
    if( pAvStream == nullptr ) {
        return sCtx.error();
    }
    PAvSample sampleMeta = pSrc->frameMeta;
    pAvStream->id = pSrc->streamingId;
    pAvStream->index = pFormatContext->nb_streams-1;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
        * of which frame timestamps are represented. For fixed-fps content,
        * timebase should be 1/framerate and timestamp increments should be
        * identical to 1. */
    pAvStream->time_base = { 1, pSrc->timeRate };
    pAvStream->duration = pSrc->timeDuration;

    AVCodecParameters *parameters = pAvStream->codecpar;
    parameters->codec_type = AVMEDIA_TYPE_VIDEO;
    parameters->codec_id = pOutputFormat->video_codec;
    parameters->width    = sampleMeta.videoWidth;
    parameters->height   = sampleMeta.videoHeight;
    parameters->format   = CAvSampleType::toPixFormat(sampleMeta.sampleFormat);
    //parameters->qu
    int iFormat = 0;
    while(pCodec->pix_fmts[iFormat] != parameters->format && pCodec->pix_fmts[iFormat] != AV_PIX_FMT_NONE) {
        iFormat++;
    }
    if(pCodec->pix_fmts[iFormat] == AV_PIX_FMT_NONE) {
        parameters->format = pCodec->pix_fmts[0];
    }

    // 创建编码上下文
    AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
    if(pCodecContext == nullptr) {
        return sCtx.error();
    }
    m_spCodecCtx.take(pCodecContext, [](AVCodecContext* pPtr) {
        avcodec_free_context(&pPtr);
    });
    if ((avcodec_parameters_to_context(pCodecContext, pAvStream->codecpar)) < 0) {
        return sCtx.error("拷贝参数到编码器中失败");
    }

    //
    // 修正CodecContext参数
    //
    {
        //pCodecContext->bit_rate = 1600000;
        pCodecContext->qmax = 5;
        pCodecContext->qmin = 1;

        /* Resolution must be a multiple of two. */
        pCodecContext->time_base = pAvStream->time_base;
        pCodecContext->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        if (pCodecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            pCodecContext->max_b_frames = 2;
        }
        if (pCodecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
                * This does not happen with normal video, it just happens here as
                * the motion of the chroma plane does not match the luma plane. */
            pCodecContext->mb_decision = 2;
        }
    }

    //
    // 创建符合要求的格式转化器
    //
    {
        PAvSample sampleMeta;
        sampleMeta.videoWidth = pCodecContext->width;
        sampleMeta.videoHeight = pCodecContext->height;
        sampleMeta.sampleFormat = (EAvSampleFormat)pCodecContext->pix_fmt;
        sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
        if( SAvFactory::getAvFactory()->openAvFrameConverter(sampleMeta, m_spConverter) != sCtx.success() ) {
            return sCtx.error();
        }
    }

    //
    // 分配AVFrame内存
    //
    AVFrame* pAVFrame = av_frame_alloc();
    if(pAVFrame == nullptr) {
        return sCtx.error();
    }
    m_pAVFrame.take(pAVFrame, [](AVFrame* pPtr){
        av_frame_free(&pPtr);
    });

    m_pAvStream = pAvStream;
    m_iStreamingId = pAvStream->id;
    m_iStreamingIndex = pAvStream->index;
    return sCtx.success();
}

int CAvOutStreaming::initAudio(AVFormatContext* pFormatContext, const PAvStreaming* pSrc) {
    const AVOutputFormat* pOutputFormat = pFormatContext->oformat;
    if(pOutputFormat == nullptr || pOutputFormat->audio_codec == AV_CODEC_ID_NONE) {
        return sCtx.error();
    }

    // 获取Codec
    AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_AC3);
    if(pCodec == nullptr) {
        return sCtx.error();
    }

    // 创建流
    AVStream* pAvStream = avformat_new_stream(pFormatContext, nullptr);
    if( pAvStream == nullptr ) {
        return sCtx.error();
    }
    pAvStream->id = pSrc->streamingId;

    // 创建编码上下文
    AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
    if(pCodecContext == nullptr) {
        return sCtx.error();
    }
    m_spCodecCtx.take(pCodecContext, [](AVCodecContext* pPtr) {
        avcodec_free_context(&pPtr);
    });

    //
    // 设置CodecContext参数
    //
    {
        PAvSample sampleMeta = pSrc->frameMeta;
        pCodecContext->codec_type = AVMEDIA_TYPE_AUDIO;
        pCodecContext->codec_id = pCodec->id;

        pCodecContext->bit_rate    = 64000;
        pCodecContext->sample_rate = sampleMeta.audioRate;
        if (pCodec->supported_samplerates) {
            pCodecContext->sample_rate = pCodec->supported_samplerates[0];
            for ( int i = 0; pCodec->supported_samplerates[i]; i++) {
                if(pCodec->supported_samplerates[i] == sampleMeta.audioRate) {
                    pCodecContext->sample_rate = sampleMeta.audioRate;
                    break;
                }
            }
        }

        int64_t channel_layout = av_get_default_channel_layout(sampleMeta.audioChannels);
        pCodecContext->channel_layout = channel_layout;
        if (pCodec->channel_layouts) {
            pCodecContext->channel_layout = pCodec->channel_layouts[0];
            for (int i = 0; pCodec->channel_layouts[i]; i++) {
                if (pCodec->channel_layouts[i] == channel_layout){
                    pCodecContext->channel_layout = channel_layout;
                }
            }
        }
        pCodecContext->channels = av_get_channel_layout_nb_channels(pCodecContext->channel_layout);
        pAvStream->time_base = { 1, pSrc->timeRate };
        pAvStream->duration = pSrc->timeDuration;

        pCodecContext->sample_fmt = CAvSampleType::toSampleFormat(sampleMeta.sampleFormat);
        //由于特定的编码器只支持特定的数据类型，所以，需要修改目标像素格式为编码器需要的格式
        int iFormat = 0;
        while(pCodec->sample_fmts[iFormat] != pCodecContext->sample_fmt && pCodec->sample_fmts[iFormat] != AV_SAMPLE_FMT_NONE) {
            iFormat++;
        }
        if(pCodec->sample_fmts[iFormat] == AV_SAMPLE_FMT_NONE) {
            pCodecContext->sample_fmt = pCodec->sample_fmts[0];
        }
    }

    //
    // 创建符合要求的格式转化器
    //
    {
        PAvSample sampleMeta;
        sampleMeta.audioChannels = pCodecContext->channels;
        sampleMeta.audioRate = pCodecContext->sample_rate;
        sampleMeta.sampleFormat = (EAvSampleFormat)pCodecContext->sample_fmt;
        sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio;
        if( SAvFactory::getAvFactory()->openAvFrameConverter(sampleMeta, m_spConverter) != sCtx.success() ) {
            return sCtx.error();
        }
    }


    //
    // 分配AVFrame内存
    //
    AVFrame* pAVFrame = av_frame_alloc();
    if(pAVFrame == nullptr) {
        return sCtx.error();
    }
    m_pAVFrame.take(pAVFrame, [](AVFrame* pPtr){
        av_frame_free(&pPtr);
    });

    m_pAvStream = pAvStream;
    m_iStreamingId = pAvStream->id;
    m_iStreamingIndex = pAvStream->index;
    return sCtx.success();
}


int CAvOutStreaming::open(AVFormatContext* pFormatContext) {

    if( avcodec_open2(m_spCodecCtx, nullptr, nullptr) < 0 ) {
        return sCtx.error();
    }

    /* copy the stream parameters to the muxer */
    int ret = avcodec_parameters_from_context(m_pAvStream->codecpar, m_spCodecCtx);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        return sCtx.error();
    }

    return sCtx.success();
}

int CAvOutStreaming::close(AVFormatContext* pFormatContext) {
    m_spCodecCtx.release();
    return sCtx.success();
}

int CAvOutStreaming::writeFrame(AVFormatContext* pFormatContext, const SAvFrame& spFrame) {

    //空值表示关闭流
    const PAvFrame* pFrame = spFrame? spFrame->getFramePtr() : nullptr;
    if( pFrame == nullptr ) {
        return writeFrame(pFormatContext, (AVFrame*)nullptr);
    }

    //只处理自己的流对应的帧
    if(pFrame->streamingId != m_iStreamingId) {
        return sCtx.success();
    }

    //转化为当前编码器可以识别的帧
    SAvFrame spOut;
    if( m_spConverter->pipeIn(spFrame, spOut) != sCtx.success() ) {
        return sCtx.error("帧格式转化失败");
    }
    pFrame = spOut->getFramePtr();

    //
    // 准备帧数据对象
    //
    PAvSample sampleMeta = pFrame->sampleMeta;
    AVFrame* pAVFrame = m_pAVFrame;
    for(int i=0; i<pFrame->nPlanes; i++) {
        pAVFrame->data[i] = pFrame->ppPlanes[i];
        pAVFrame->linesize[i] = pFrame->pPlaneLineSizes[i];
    }
    pAVFrame->pts = pFrame->timeStamp;
    pAVFrame->extended_data = pAVFrame->data;

    //视频
    switch (sampleMeta.sampleType)
    {
    case EAvSampleType::AvSampleType_Video:
        pAVFrame->width = sampleMeta.videoWidth;
        pAVFrame->height = sampleMeta.videoHeight;
        pAVFrame->format = CAvSampleType::toPixFormat(sampleMeta.sampleFormat);
        break;

    case EAvSampleType::AvSampleType_Audio:
        pAVFrame->nb_samples = pFrame->nWidth;
        pAVFrame->sample_rate = sampleMeta.audioRate;
        pAVFrame->channels = sampleMeta.audioChannels;
        pAVFrame->extended_data = &pAVFrame->data[0];
        pAVFrame->format = CAvSampleType::toSampleFormat(sampleMeta.sampleFormat);
        break;
    
    default:
        break;
    }
 
    return writeFrame(pFormatContext, pAVFrame);
}

int CAvOutStreaming::writeFrame(AVFormatContext* pFormatContext, AVFrame* pAVFrame) {
    
    //
    // 压缩数据
    //
    int ret = avcodec_send_frame(m_spCodecCtx, pAVFrame);
    if( ret < 0 ) {
        return sCtx.error();
    }

    //
    // 创建Package
    //
    AVPacket* pPkt = av_packet_alloc();
    if(pPkt == nullptr) {
        return sCtx.error();
    }
    CTaker<AVPacket*> takePkg(pPkt, [](AVPacket* pPtr){
        av_packet_free(&pPtr);
    });


    //
    // 循环读取压缩包，并写入数据，直到
    //
    while(true) {

        int ret = avcodec_receive_packet(m_spCodecCtx, takePkg);
        switch(ret) {
        case 0:
            break;

        case AVERROR(EAGAIN):
        case AVERROR_EOF:
            return sCtx.success();

        default:
            return sCtx.error();
        }

        //
        // 设置Package参数
        //
        pPkt->stream_index = m_iStreamingIndex;
        //av_packet_rescale_ts(pPkt, m_spCodecCtx->time_base, m_pAvStream->time_base);
        if( av_interleaved_write_frame(pFormatContext, takePkg) < 0 ) {
            return sCtx.error();
        }
    }
    return sCtx.error();
}

FFMPEG_NAMESPACE_LEAVE