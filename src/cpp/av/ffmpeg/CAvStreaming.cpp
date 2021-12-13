#include "av_ffmpeg.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

CAvStreaming::CAvStreaming() {
    m_eAvStreamingType = SAvFrame::AVSTREAMTYPE_UNKNOWN;
    m_iStreamingIndex = -1;

    m_pImagePointers[0] = nullptr;
}

CAvStreaming::~CAvStreaming() {
    release();
}

void CAvStreaming::release() {
    releaseAudioCtx();
    releaseVideoCtx();
}

SAvFrame::AvFrameType CAvStreaming::getFrameType() {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return SAvFrame::AVSTREAMTYPE_VIDEO;
        
        case AVMEDIA_TYPE_AUDIO:
            return SAvFrame::AVSTREAMTYPE_AUDIO;
        }
    }
    return SAvFrame::AVSTREAMTYPE_UNKNOWN;
}

int CAvStreaming::getStreamingId() {
    return m_iStreamingIndex;
}

int CAvStreaming::init(AVStream* pAvStream, int iStreamingIndex) {
    AVCodecParameters* pCodecParameter = pAvStream->codecpar;
    if(pCodecParameter == nullptr) {
        return Error::ERRORTYPE_FAILURE;
    }

    CTaker<AVCodecContext*> pCodecCtx(  avcodec_alloc_context3(nullptr), 
                                        [](AVCodecContext* pCtx){
                                            avcodec_free_context(&pCtx);
                                        });
    if( avcodec_parameters_to_context(pCodecCtx, pCodecParameter) < 0 ) {
        return Error::ERRORTYPE_FAILURE;
    }

    AVCodec* pCodec=avcodec_find_decoder(pCodecParameter->codec_id);
    if(pCodec==NULL){
        printf("Codec not found.\n");
        return Error::ERRORTYPE_FAILURE;
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
        printf("Could not open codec.\n");
        return Error::ERRORTYPE_FAILURE;
    }

    m_spCodecCtx.take(pCodecCtx);
    m_iStreamingIndex = iStreamingIndex;
    return Error::ERRORTYPE_SUCCESS;
}

void CAvStreaming::releaseAudioCtx() {
    m_spSwrCtx.release();
}

STensor CAvStreaming::convertAudio(AVFrame* pAvFrame, AVSampleFormat eSampleFormat, int nSampleRate, int nChannels) {

    //
    // 如果格式相同，则直接读取并返回帧数据
    //
    if( pAvFrame->sample_rate == nSampleRate && 
        pAvFrame->channels == nChannels &&
        pAvFrame->format == eSampleFormat ) {

        // 根据相应音频参数，获得所需缓冲区大小
        int nFrameSize = av_samples_get_buffer_size(
                NULL, 
                pAvFrame->channels,
                pAvFrame->nb_samples,
                eSampleFormat,
                1);
            
        return STensor::createVector(nFrameSize, (unsigned char*)pAvFrame->data[0]);
    }

    if( m_spSwrCtx ) {
        if( nSampleRate != m_nCtxSampleRate || 
            nChannels != m_nCtxChannels ||
            eSampleFormat != m_eCtxSampleFormat ) {
            releaseAudioCtx();
        }
    }

    if( !m_spSwrCtx  ) {
        
        int64_t nChannelLayout = av_get_default_channel_layout(nChannels);
        int64_t nFrameChannelLayout = av_get_default_channel_layout(pAvFrame->channels);

        m_spSwrCtx.take(swr_alloc_set_opts(
                                    NULL,
                                    nChannelLayout, 
                                    eSampleFormat, 
                                    nSampleRate,
                                    nFrameChannelLayout,           
                                    (AVSampleFormat)pAvFrame->format, 
                                    pAvFrame->sample_rate,
                                    0,
                                    NULL),
                        [](SwrContext* pCtx){swr_free(&pCtx);});
        if( !m_spSwrCtx ) {
            return STensor();
        }

        if( swr_init(m_spSwrCtx) < 0 ) {
            releaseAudioCtx();
            return STensor();
        }

        m_nCtxSampleRate = nSampleRate;
        m_nCtxChannels = nChannels;
        m_eCtxSampleFormat = eSampleFormat;
    }


    // 重采样输出参数1：输出音频缓冲区尺寸
    // 重采样输出参数2：输出音频缓冲区
    uint8_t *out_buf = nullptr;
    unsigned int out_buf_size = 0;
    uint8_t **out = &out_buf;

    // 重采样输出参数：输出音频样本数(多加了256个样本)
    int out_count = (int64_t)pAvFrame->nb_samples * nSampleRate / pAvFrame->sample_rate + 256;
    // 重采样输出参数：输出音频缓冲区尺寸(以字节为单位)
    int nBufSize  = av_samples_get_buffer_size(NULL, nChannels, out_count, m_eCtxSampleFormat, 0);
    if (nBufSize < 0)
    {
        printf("av_samples_get_buffer_size() failed\n");
        releaseAudioCtx();
        return STensor();
    }
    av_fast_malloc(&out_buf, &out_buf_size, nBufSize);

    // 重采样输入参数1：输入音频样本数是p_frame->nb_samples
    // 重采样输入参数2：输入音频缓冲区
    const uint8_t **in = (const uint8_t **)pAvFrame->extended_data;
    int in_count = pAvFrame->nb_samples;

    // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
    int nb_samples = swr_convert(m_spSwrCtx, out, out_count, in, pAvFrame->nb_samples);
    if (nb_samples < 0) {
        printf("swr_convert() failed\n");
        releaseAudioCtx();
        return STensor();
    }
    if (nb_samples == out_count)
    {
        printf("audio buffer is probably too small\n");
        //if (swr_init(s_audio_swr_ctx) < 0)
        //    swr_free(&s_audio_swr_ctx);
    }

    // 重采样返回的一帧音频数据大小(以字节为单位)
    int nData = nb_samples * nChannels * av_get_bytes_per_sample(eSampleFormat);
    return STensor::createVector(nData, (unsigned char*)out_buf);
}

void CAvStreaming::releaseVideoCtx() {
    if(m_pImagePointers[0] != nullptr) {
        av_freep(&m_pImagePointers[0]);
        m_pImagePointers[0] = nullptr;
    }

    m_spSwsContext.release();
}

STensor CAvStreaming::convertImage(AVFrame* pAvFrame, AVPixelFormat ePixFormat) {
    //如果上次用的像素格式与这次不同，则释放上次的转化器，重新创建
    if( m_spSwsContext && m_ePixFormat != ePixFormat ) {
        releaseVideoCtx();
    }
    
    if( !m_spSwsContext ) {
        
        switch(ePixFormat) {
        case AV_PIX_FMT_RGBA:
            m_nPixBytes = 4;
            break;

        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:
            m_nPixBytes = 3;
            break;
        
        default:
            return STensor();
        }

        //
        SwsContext* pSwsContext = sws_getContext(
            //源图像的 宽 , 高 , 图像像素格式
            m_spCodecCtx->width, m_spCodecCtx->height, m_spCodecCtx->pix_fmt,
            //目标图像 大小不变 , 不进行缩放操作 , 只将像素格式设置成 RGBA 格式的
            m_spCodecCtx->width, m_spCodecCtx->height, ePixFormat,
            //使用的转换算法 , FFMPEG 提供了许多转换算法 , 有快速的 , 有高质量的 , 需要自己测试
            SWS_BILINEAR,
            //源图像滤镜 , 这里传 NULL 即可
            0,
            //目标图像滤镜 , 这里传 NULL 即可
            0,
            //额外参数 , 这里传 NULL 即可
            0
        );
        if(pSwsContext == nullptr) {
            return STensor();
        }

        m_spSwsContext.take(pSwsContext, sws_freeContext);
        if( av_image_alloc(m_pImagePointers, m_pLinesizes,
            m_spCodecCtx->width, m_spCodecCtx->height, ePixFormat, 1) < 0 ){
            releaseVideoCtx();
            return STensor();
        }
        m_ePixFormat = ePixFormat;
    }

    sws_scale(
        //SwsContext *swsContext 转换上下文
        m_spSwsContext,
        //要转换的数据内容
        pAvFrame->data,
        //数据中每行的字节长度
        pAvFrame->linesize,
        0,
        pAvFrame->height,
        //转换后目标图像数据存放在这里
        m_pImagePointers,
        //转换后的目标图像行数
        m_pLinesizes
    );

    //
    //  修正视频宽度。及pAvFrame->width != m_pLinesizes[0]/4时，以后者为准。如果返
    //  回值中的视频，一行的字节数不等于视频宽度*4(RGBA)，则这个时候视频数据有点难处
    //  理，只能暂时将返回图像的宽度扩大(有垃圾数据)或缩小(数据丢失）到返回的实际宽度。
    //  如果想调整为实际视频大小，则需要对每一行的数据做处理，性能太低。
    //  
    //  这个地方可以考虑优化，因为所有帧只需要一个统一的Dim即可，无需每次创建，前提是
    //  width总是相同。
    //
    int width = m_pLinesizes[0]/m_nPixBytes;
    int dimsize[3] = { width, pAvFrame->height, m_nPixBytes };
    STensor spDimTensor = STensor::createVector(3, dimsize);
    return STensor::createTensor(spDimTensor, width*pAvFrame->height*m_nPixBytes, m_pImagePointers[0]);
}

FFMPEG_NAMESPACE_LEAVE