#include "av_ffmpeg.h"
#include "CAvStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

CAvStreaming::CAvStreaming() {
    m_pAvStream = nullptr;
    m_eAvStreamingType = EAvStreamingType::AvStreamingType_None;
    m_iStreamingIndex = -1;
    m_pData[0] = nullptr;
    m_lastMeta.eSampleType = EAvSampleType::AvSampleType_None;
}

CAvStreaming::~CAvStreaming() {
    release();
}

void CAvStreaming::release() {
    releaseAudioCtx();
    releaseVideoCtx();
}

EAvStreamingType CAvStreaming::getStreamingType() {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return EAvStreamingType::AvStreamingType_Video;
        
        case AVMEDIA_TYPE_AUDIO:
            return EAvStreamingType::AvStreamingType_Audio;
        }
    }
    return EAvStreamingType::AvStreamingType_None;
}

int CAvStreaming::getStreamingId() {
    return m_iStreamingIndex;
}


int CAvStreaming::getSampleRate() {
    return m_spCodecCtx->sample_rate;
}

EAvSampleType CAvStreaming::getSampleType() {
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            EAvSampleType eType = CAvSampleType::convert(m_spCodecCtx->pix_fmt);
            if(eType == EAvSampleType::AvSampleType_None) {
                eType = EAvSampleType::AvSampleType_Video_RGB;
            }
            return eType;
        }
        break;

    case AVMEDIA_TYPE_AUDIO:
        {
            EAvSampleType eType = CAvSampleType::convert(m_spCodecCtx->sample_fmt);
            if(eType == EAvSampleType::AvSampleType_None) {
                eType = EAvSampleType::AvSampleType_Audio_S16;
            }
            return eType;
        }
        break;
    }
    return EAvSampleType::AvSampleType_None;
}

const CAvSampleMeta& CAvStreaming::getSampleMeta() {
    return m_sampleMeta;
}

int CAvStreaming::setSampleMeta(const CAvSampleMeta& sampleMeta) {
    m_sampleMeta = sampleMeta;
    releaseAudioCtx();
    releaseVideoCtx();
    return Error::ERRORTYPE_SUCCESS;
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
    m_pAvStream = pAvStream;
    m_sampleMeta.eSampleType = getSampleType();
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            m_sampleMeta.nVideoWidth = m_spCodecCtx->width;
            m_sampleMeta.nVideoHeight = m_spCodecCtx->height;
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        {
            m_sampleMeta.nAudioRate = m_spCodecCtx->sample_rate;
            m_sampleMeta.nAudioChannels = m_spCodecCtx->channels;
        }
        break;
    }
    return Error::ERRORTYPE_SUCCESS;
}

void CAvStreaming::releaseAudioCtx() {
    m_spSwrCtx.release();
}

int CAvStreaming::convertToTensor(STensor& spData, AVFrame* pAvFrame) {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return convertImage(spData, pAvFrame);
        
        case AVMEDIA_TYPE_AUDIO:
            return convertAudio(spData, pAvFrame);
        }
    }
    return Error::ERRORTYPE_FAILURE;
}

int CAvStreaming::convertAudio(STensor& spTensor, AVFrame* pAvFrame) {
    CAvSampleMeta& sampleMeta = m_sampleMeta;
    int nChannels = sampleMeta.nAudioChannels;
    int nAudioRate = sampleMeta.nAudioRate;
    AVSampleFormat eSampleFormat = CAvSampleType::toSampleFormat(sampleMeta.eSampleType);
    //
    // 如果格式相同，则直接读取并返回帧数据
    //
    if( pAvFrame->sample_rate == nAudioRate && 
        pAvFrame->channels == nChannels &&
        pAvFrame->format == eSampleFormat ) {

        // 根据相应音频参数，获得所需缓冲区大小
        int nFrameSize = av_samples_get_buffer_size(
                NULL, 
                nChannels,
                pAvFrame->nb_samples,
                eSampleFormat,
                1);
        
        spTensor = STensor::createVector(pAvFrame->nb_samples, (unsigned char*)pAvFrame->data[0]);
        return Error::ERRORTYPE_SUCCESS;
    }

    if( m_spSwrCtx ) {
        if( m_lastMeta.nAudioRate != sampleMeta.nAudioRate || 
            m_lastMeta.nAudioChannels != sampleMeta.nAudioChannels ||
            m_lastMeta.eSampleType != sampleMeta.eSampleType ) {
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
                                    nAudioRate,
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
            return Error::ERRORTYPE_FAILURE;
        }

        m_lastMeta = sampleMeta;
    }

    // 重采样输出参数1：输出音频缓冲区尺寸
    // 重采样输出参数2：输出音频缓冲区
    uint8_t *out_buf = nullptr;
    unsigned int out_buf_size = 0;
    uint8_t **out = &out_buf;

    // 重采样输出参数：输出音频样本数(多加了256个样本)
    int out_count = (int64_t)pAvFrame->nb_samples * nAudioRate / pAvFrame->sample_rate + 256;
    // 重采样输出参数：输出音频缓冲区尺寸(以字节为单位)
    int nBufSize  = av_samples_get_buffer_size(NULL, nChannels, out_count, eSampleFormat, 0);
    if (nBufSize < 0)
    {
        printf("av_samples_get_buffer_size() failed\n");
        releaseAudioCtx();
        return Error::ERRORTYPE_FAILURE;
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
    }

    // 重采样返回的一帧音频数据大小(以字节为单位)
    int nData = nb_samples * nChannels * av_get_bytes_per_sample(eSampleFormat);
    spTensor = STensor::createVector(nData, (unsigned char*)out_buf);
    return Error::ERRORTYPE_FAILURE;
}

void CAvStreaming::releaseVideoCtx() {
    if(m_pData[0] != nullptr) {
        av_freep(&m_pData[0]);
        m_pData[0] = nullptr;
    }

    m_spSwsContext.release();
}

int CAvStreaming::convertImage(STensor& spTensor, AVFrame* pAvFrame) {
    CAvSampleMeta& sampleMeta = m_sampleMeta;
    int nTargetWidth = sampleMeta.nVideoWidth > 0 ? sampleMeta.nVideoWidth : pAvFrame->width;
    int nTargetHeight = sampleMeta.nVideoHeight > 0 ? sampleMeta.nVideoHeight : pAvFrame->height;
    AVPixelFormat eTargetPixelFormat = CAvSampleType::toPixFormat(sampleMeta.eSampleType);

    //如果上次用的像素格式与这次不同，则释放上次的转化器，重新创建
    if( m_spSwsContext && (
            m_lastMeta.eSampleType != sampleMeta.eSampleType ||
            m_lastMeta.nVideoHeight != nTargetWidth ||
            m_lastMeta.nVideoWidth != nTargetHeight
            ) ) {
        releaseVideoCtx();
    }
    
    if( !m_spSwsContext ) {
        
        //
        SwsContext* pSwsContext = sws_getContext(
            //源图像的 宽 , 高 , 图像像素格式
            m_spCodecCtx->width, m_spCodecCtx->height, m_spCodecCtx->pix_fmt,
            //目标图像 大小不变 , 不进行缩放操作 , 只将像素格式设置成 RGBA 格式的
            nTargetWidth, nTargetHeight, eTargetPixelFormat,
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
            return Error::ERRORTYPE_FAILURE;
        }

        m_spSwsContext.take(pSwsContext, sws_freeContext);
        if( av_image_alloc(m_pData, m_pLinesizes,
            nTargetWidth, nTargetHeight, eTargetPixelFormat, 1) < 0 ){
            releaseVideoCtx();
            return Error::ERRORTYPE_FAILURE;
        }

        switch(sampleMeta.eSampleType) {
        case EAvSampleType::AvSampleType_Video_RGBA:
            m_nPixelBytes = 4;
            break;

        case EAvSampleType::AvSampleType_Video_RGB:
            m_nPixelBytes = 3;
            break;
        }
        m_lastMeta = sampleMeta;
        int dimsize[3] = { nTargetWidth, nTargetHeight, m_nPixelBytes };
        m_spLastDimTensor = STensor::createVector(3, dimsize);
    }

    int ret_height = sws_scale(
        //SwsContext *swsContext 转换上下文
        m_spSwsContext,
        //要转换的数据内容
        pAvFrame->data,
        //数据中每行的字节长度
        pAvFrame->linesize,
        0,
        pAvFrame->height,
        //转换后目标图像数据存放在这里
        m_pData,
        //转换后的目标图像行数
        m_pLinesizes
    );

    // 如果转化结果尺寸与想要的目标尺寸不一致，则转化失败
    if( m_pLinesizes[0] / m_nPixelBytes != nTargetWidth ||
        ret_height != nTargetHeight ) {
            return Error::ERRORTYPE_FAILURE;
    }

    //
    //  修正视频宽度。及nTargetWidth != m_pLinesizes[0]/4时，以后者为准。如果返
    //  回值中的视频，一行的字节数不等于视频宽度*4(RGBA)，则这个时候视频数据有点难处
    //  理，只能暂时将返回图像的宽度扩大(有垃圾数据)或缩小(数据丢失）到返回的实际宽度。
    //  如果想调整为实际视频大小，则需要对每一行的数据做处理，性能太低。
    //
    spTensor = STensor::createTensor(m_spLastDimTensor, nTargetWidth*nTargetHeight*m_nPixelBytes, m_pData[0]);
    return Error::ERRORTYPE_SUCCESS;
}

FFMPEG_NAMESPACE_LEAVE