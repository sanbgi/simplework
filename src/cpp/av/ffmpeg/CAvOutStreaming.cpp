#include "av_ffmpeg.h"
#include "CAvOutStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

CAvOutStreaming::CAvOutStreaming() {
    m_pAvStream = nullptr;
    m_eStreamingType = EAvStreamingType::AvStreamingType_None;
    m_iStreamingId = -1;
    m_pCodec = nullptr;
    m_nWriteNumber = 0;
}

CAvOutStreaming::~CAvOutStreaming() {
    release();
}

void CAvOutStreaming::release() {
}

EAvStreamingType CAvOutStreaming::getStreamingType() {
    return m_eStreamingType;
}

int CAvOutStreaming::getStreamingId() {
    return m_iStreamingId;
}

int CAvOutStreaming::getTimeRate() {
    return m_nTimeRate;
}

const SAvSampleMeta& CAvOutStreaming::getSampleMeta() {
    return m_sampleMeta;
}

int CAvOutStreaming::init(AVFormatContext* pFormatContext, SAvStreaming& src) {
    switch(src->getStreamingType()) {
    case EAvStreamingType::AvStreamingType_Video:
        return initVideo(pFormatContext, src);

    case EAvStreamingType::AvStreamingType_Audio:
        return initAudio(pFormatContext, src);
    }
    return SError::ERRORTYPE_FAILURE;
}

int CAvOutStreaming::initVideo(AVFormatContext* pFormatContext, SAvStreaming& src) {

    m_nTimeRate = 24;

    const AVOutputFormat* pOutputFormat = pFormatContext->oformat;
    if(pOutputFormat == nullptr || pOutputFormat->video_codec == AV_CODEC_ID_NONE) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 获取Codec
    AVCodec* pCodec = avcodec_find_encoder(pOutputFormat->video_codec);
    if(pCodec == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 创建流
    AVStream* pAvStream = avformat_new_stream(pFormatContext, nullptr);
    if( pAvStream == nullptr ) {
        return SError::ERRORTYPE_FAILURE;
    }
    pAvStream->id = src->getStreamingId();
    pAvStream->index = pFormatContext->nb_streams-1;

    // 创建编码上下文
    AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
    if(pCodecContext == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_spCodecCtx.take(pCodecContext, [](AVCodecContext* pPtr) {
        avcodec_free_context(&pPtr);
    });

    //
    // 设置CodecContext参数
    //
    {
        SAvSampleMeta sampleMeta = src->getSampleMeta();
        pCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
        pCodecContext->codec_id = pOutputFormat->video_codec;
        //pCodecContext->bit_rate = 1600000;
        pCodecContext->qmax = 5;
        pCodecContext->qmin = 1;

        /* Resolution must be a multiple of two. */
        pCodecContext->width    = sampleMeta.videoWidth;
        pCodecContext->height   = sampleMeta.videoHeight;

        /* timebase: This is the fundamental unit of time (in seconds) in terms
            * of which frame timestamps are represented. For fixed-fps content,
            * timebase should be 1/framerate and timestamp increments should be
            * identical to 1. */
        pAvStream->time_base = { 1, src->getTimeRate() };
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

        pCodecContext->pix_fmt = CAvSampleType::toPixFormat(sampleMeta.sampleType);
        int iFormat = 0;
        while(pCodec->pix_fmts[iFormat] != pCodecContext->pix_fmt && pCodec->pix_fmts[iFormat] != AV_PIX_FMT_NONE) {
            iFormat++;
        }
        if(pCodec->pix_fmts[iFormat] == AV_PIX_FMT_NONE) {
            pCodecContext->pix_fmt = pCodec->pix_fmts[0];
        }
    }

    //
    // 分配AVFrame内存
    //
    AVFrame* pAVFrame = av_frame_alloc();
    if(pAVFrame == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_pAVFrame.take(pAVFrame, [](AVFrame* pPtr){
        av_frame_free(&pPtr);
    });

    m_pAvStream = pAvStream;
    m_iStreamingId = pAvStream->id;
    m_iStreamingIndex = pAvStream->index;
    return SError::ERRORTYPE_SUCCESS;
}

int CAvOutStreaming::initAudio(AVFormatContext* pFormatContext, SAvStreaming& src) {
    const AVOutputFormat* pOutputFormat = pFormatContext->oformat;
    if(pOutputFormat == nullptr || pOutputFormat->audio_codec == AV_CODEC_ID_NONE) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 获取Codec
    AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_AC3);
    if(pCodec == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 创建流
    AVStream* pAvStream = avformat_new_stream(pFormatContext, nullptr);
    if( pAvStream == nullptr ) {
        return SError::ERRORTYPE_FAILURE;
    }
    pAvStream->id = src->getStreamingId();

    // 创建编码上下文
    AVCodecContext* pCodecContext = avcodec_alloc_context3(pCodec);
    if(pCodecContext == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_spCodecCtx.take(pCodecContext, [](AVCodecContext* pPtr) {
        avcodec_free_context(&pPtr);
    });

    //
    // 设置CodecContext参数
    //
    {
        SAvSampleMeta sampleMeta = src->getSampleMeta();
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
        pAvStream->time_base = (AVRational){ 1, sampleMeta.audioRate };

        pCodecContext->sample_fmt = CAvSampleType::toSampleFormat(sampleMeta.sampleType);
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
    // 分配AVFrame内存
    //
    AVFrame* pAVFrame = av_frame_alloc();
    if(pAVFrame == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_pAVFrame.take(pAVFrame, [](AVFrame* pPtr){
        av_frame_free(&pPtr);
    });

    m_pAvStream = pAvStream;
    m_iStreamingId = pAvStream->id;
    m_iStreamingIndex = pAvStream->index;
    return SError::ERRORTYPE_SUCCESS;
}

int CAvOutStreaming::open(AVFormatContext* pFormatContext) {

    if( avcodec_open2(m_spCodecCtx, m_pCodec, nullptr) < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    /* copy the stream parameters to the muxer */
    int ret = avcodec_parameters_from_context(m_pAvStream->codecpar, m_spCodecCtx);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        return SError::ERRORTYPE_FAILURE;
    }

    return SError::ERRORTYPE_SUCCESS;
}

int CAvOutStreaming::close(AVFormatContext* pFormatContext) {
    m_spCodecCtx.release();
    return SError::ERRORTYPE_SUCCESS;
}

int CAvOutStreaming::writeFrame(AVFormatContext* pFormatContext, const SAvFrame& rFrame) {
    if(rFrame) {
        SAvStreaming& rStreaming = rFrame->getStreaming();
        if(rStreaming->getStreamingId() == m_iStreamingId ) {
            switch(rStreaming->getStreamingType()) {
            case EAvStreamingType::AvStreamingType_Video:
                return writeVideoFrame(pFormatContext, rFrame);

            case EAvStreamingType::AvStreamingType_Audio:
                return writeAudioFrame(pFormatContext, rFrame);
            }
        }
        return SError::ERRORTYPE_SUCCESS;
    }
    return writeFrame(pFormatContext, nullptr);
}

int CAvOutStreaming::writeVideoFrame(AVFormatContext* pFormatContext, const SAvFrame& rFrame) {

    AVCodecContext* pCodecContext = m_spCodecCtx;
    //
    // 准备数据
    //
    STensor spTensor = rFrame->getData();
    if( !spTensor ) {
        return SError::ERRORTYPE_FAILURE;
    }
    STensor spDims = spTensor->getDimVector();
    if( !spDims || spDims->getDataSize() != 3) {
        return SError::ERRORTYPE_FAILURE;
    }
    const int* pDims = spDims->getDataPtr<int>();
    if( pDims[0] != pCodecContext->height || pDims[1] != pCodecContext->width ) {
        return SError::ERRORTYPE_FAILURE;
    }

    //
    // 准备帧数据对象
    //
    SAvSampleMeta sampleMeta = rFrame->getStreaming()->getSampleMeta();
    AVFrame* pAVFrame = m_pAVFrame;
    pAVFrame->data[0] = (uint8_t*)spTensor->getDataPtr<unsigned char>();
    pAVFrame->linesize[0] = pDims[1]*pDims[2];
    pAVFrame->pts = rFrame->getTimeStamp();
    pAVFrame->width = sampleMeta.videoWidth;
    pAVFrame->height = sampleMeta.videoHeight;
    pAVFrame->format = CAvSampleType::toPixFormat(sampleMeta.sampleType);
    if(m_converter.convert(pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt, *pAVFrame) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }

    pAVFrame->format = pCodecContext->pix_fmt;
    pAVFrame->width = pCodecContext->width;
    pAVFrame->height = pCodecContext->height;
    for(int i=0; i<AV_NUM_DATA_POINTERS; i++) {
        pAVFrame->data[i] = m_converter.m_pVideoData[i];
        pAVFrame->linesize[i] = m_converter.m_pVideoLinesizes[i];
    }
    return writeFrame(pFormatContext, pAVFrame);
}

int CAvOutStreaming::writeAudioFrame(AVFormatContext* pFormatContext, const SAvFrame& rFrame) {
    
    AVCodecContext* pCodecContext = m_spCodecCtx;
    //
    // 准备数据
    //
    STensor spTensor = rFrame->getData();
    if( !spTensor ) {
        return SError::ERRORTYPE_FAILURE;
    }
    STensor spDims = spTensor->getDimVector();
    if( !spDims || spDims->getDataSize() != 3) {
        return SError::ERRORTYPE_FAILURE;
    }

    const int* pDims = spDims->getDataPtr<int>();
    if( pDims[0] != pCodecContext->channels ) {
        return SError::ERRORTYPE_FAILURE;
    }

    //
    // 准备帧数据对象
    //
    SAvSampleMeta sampleMeta = rFrame->getStreaming()->getSampleMeta();
    AVFrame* pAVFrame = m_pAVFrame;
    pAVFrame->data[0] = (uint8_t*)spTensor->getDataPtr<unsigned char>();
    pAVFrame->linesize[0] = pDims[1]*pDims[2];
    pAVFrame->nb_samples = pDims[1];
    pAVFrame->pts = rFrame->getTimeStamp();
    pAVFrame->sample_rate = sampleMeta.audioRate;
    pAVFrame->channels = sampleMeta.audioChannels;
    pAVFrame->format = CAvSampleType::toSampleFormat(sampleMeta.sampleType);
    pAVFrame->extended_data = &pAVFrame->data[0];
    if(m_converter.convert(pCodecContext->sample_rate, pCodecContext->channels, pCodecContext->sample_fmt, *pAVFrame) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }
    pAVFrame->format = pCodecContext->sample_fmt;
    pAVFrame->sample_rate = pCodecContext->sample_rate;
    pAVFrame->channels = pCodecContext->channels;
    pAVFrame->channel_layout = pCodecContext->channel_layout;
    pAVFrame->extended_data = m_converter.m_ppAudioData;
    //注意这里面只针对非planar audio对pData赋值，如果是plannar audio，pData里面的值是不完整的，只有一个通道的值
    pAVFrame->data[0] = *m_converter.m_ppAudioData; 
    pAVFrame->nb_samples = m_converter.m_nAudioSamples;
    return writeFrame(pFormatContext, pAVFrame);
}

int CAvOutStreaming::writeFrame(AVFormatContext* pFormatContext, AVFrame* pAVFrame) {
    
    //
    // 压缩数据
    //
    int ret = avcodec_send_frame(m_spCodecCtx, pAVFrame);
    if( ret < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    //
    // 创建Package
    //
    AVPacket* pPkt = av_packet_alloc();
    if(pPkt == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }

    //
    //  文档上说：如果avcodec_encode_video2调用失败，pPkt会被avcodec_encode_video2释放
    //
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
            return SError::ERRORTYPE_SUCCESS;

        default:
            return SError::ERRORTYPE_FAILURE;
        }

        //
        // 设置Package参数
        //
        pPkt->stream_index = m_iStreamingIndex;
        //av_packet_rescale_ts(pPkt, m_spCodecCtx->time_base, m_pAvStream->time_base);
        if( av_interleaved_write_frame(pFormatContext, takePkg) < 0 ) {
            return SError::ERRORTYPE_FAILURE;
        }
    }
    return SError::ERRORTYPE_FAILURE;
}

FFMPEG_NAMESPACE_LEAVE