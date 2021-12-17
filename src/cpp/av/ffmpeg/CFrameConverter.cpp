#include "av_ffmpeg.h"
#include "CFrameConverter.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

CFrameConverter::CFrameConverter(){

    //表示缓存还未初始化
    m_pVideoData[0] = nullptr;
    m_ppAudioData = nullptr;
}

CFrameConverter::~CFrameConverter() {
    releaseVideoCtx();
    releaseAudioCtx();
}

void CFrameConverter::releaseVideoCtx() {
    releaseVideoData();
    m_spSwsContext.release();
}

void CFrameConverter::releaseAudioCtx() {
    releaseAudioData();
    m_spSwrCtx.release();
}

void CFrameConverter::releaseVideoData() {
    if(m_pVideoData[0] != nullptr) {
        av_freep(&m_pVideoData[0]);
        m_pVideoData[0] = nullptr;
    }
}

void CFrameConverter::releaseAudioData() {
    if(m_ppAudioData) {
        //av_freep(*m_ppAudioData);
        av_freep(m_ppAudioData);
        m_ppAudioData = nullptr;
    }
}

int CFrameConverter::convert(int nTargetWidth, int nTargetHeight, AVPixelFormat eTargetFormat, AVFrame& src) {
    CFormat targetFormat(nTargetWidth, nTargetHeight, eTargetFormat);
    CFormat sourceFormat(src.width,src.height,(AVPixelFormat)src.format);
    if(m_spSwsContext) {
        if( m_lastTargetFormat != targetFormat || m_lastSourceFormat != sourceFormat ){
            releaseVideoCtx();
        }
    }

    if(!m_spSwsContext) {
        //
        // 创建转化器
        //
        SwsContext* pSwsContext = sws_getContext(
            //源图像的 宽 , 高 , 图像像素格式
            src.width, src.height, (AVPixelFormat)src.format,
            //目标图像 大小不变 , 不进行缩放操作 , 只将像素格式设置成 RGBA 格式的
            nTargetWidth, nTargetHeight, eTargetFormat,
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
            return SError::ERRORTYPE_FAILURE;
        }

        m_spSwsContext.take(pSwsContext, sws_freeContext);
        if( av_image_alloc(m_pVideoData, m_pVideoLinesizes,
            nTargetWidth, nTargetHeight, eTargetFormat, 1) < 0 ){
            releaseVideoCtx();
            return SError::ERRORTYPE_FAILURE;
        }

        m_lastSourceFormat = sourceFormat;
        m_lastTargetFormat = targetFormat;
    }

    int ret_height = sws_scale(
        //SwsContext *swsContext 转换上下文
        m_spSwsContext,
        //要转换的数据内容
        src.data,
        //数据中每行的字节长度
        src.linesize,
        0,
        src.height,
        //转换后目标图像数据存放在这里
        m_pVideoData,
        //转换后的目标图像行数
        m_pVideoLinesizes
    );

    // 如果转化结果尺寸与想要的目标尺寸不一致，则转化失败
    if( ret_height != nTargetHeight ) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据
    for( int i=0; i<AV_NUM_DATA_POINTERS && m_pVideoLinesizes[i]; i++ ) {
        m_nPlanes = i;
    }

    return SError::ERRORTYPE_SUCCESS;
}


int CFrameConverter::convert(int nTargetRate, int nTargetChannels, AVSampleFormat eTargetFormat, AVFrame& src) {

    CFormat targetFormat(nTargetRate, nTargetChannels, eTargetFormat);
    CFormat sourceFormat(src.sample_rate,src.channels,(AVSampleFormat)src.format);
    if( m_spSwrCtx ) {
        if( m_lastTargetFormat != targetFormat || m_lastSourceFormat != sourceFormat ){
            releaseAudioCtx();
        }
    }

    if( !m_spSwrCtx  ) {
        
        int64_t nTargetLayout = av_get_default_channel_layout(nTargetChannels);
        int64_t nFrameChannelLayout = av_get_default_channel_layout(src.channels);
        m_spSwrCtx.take(swr_alloc_set_opts(
                                    NULL,
                                    nTargetLayout, 
                                    eTargetFormat, 
                                    nTargetRate,
                                    nFrameChannelLayout,           
                                    (AVSampleFormat)src.format, 
                                    src.sample_rate,
                                    0,
                                    NULL),
                        [](SwrContext* pCtx){
                            swr_free(&pCtx);
        });
        if( !m_spSwrCtx ) {
            return SError::ERRORTYPE_FAILURE;
        }

        if( swr_init(m_spSwrCtx) < 0 ) {
            releaseAudioCtx();
            return SError::ERRORTYPE_FAILURE;
        }

        m_lastTargetFormat = targetFormat;
        m_lastSourceFormat = sourceFormat;
    }

    releaseAudioData();

    int nBufSize;
    int nTargetSamples = (int64_t)src.nb_samples * nTargetRate / src.sample_rate + 256;
    if( nBufSize = av_samples_alloc_array_and_samples( &m_ppAudioData, nullptr,
                        nTargetChannels, nTargetSamples, 
                        eTargetFormat, 0) <0 ){
        releaseVideoCtx();
        return SError::ERRORTYPE_FAILURE;
    }
    
    //
    // 这里面关系比较复杂，extended_data与pData格式相同，一般情况下也是相同的，其含义是
    //  对于planar audio，都是指向指针数组的指针，其中数组中每一个指针指向一个channel的数据
    //  对于package audio，同样是指向指针数组额指针，不过，指针数组中，只有第一个指针有效，指向具体数据
    //
    const uint8_t **in = (const uint8_t **)src.extended_data;

    // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
    int nb_samples = swr_convert(m_spSwrCtx, m_ppAudioData, nTargetSamples, in, src.nb_samples);
    if (nb_samples < 0) {
        printf("swr_convert() failed\n");
        releaseAudioCtx();
        return SError::ERRORTYPE_FAILURE;
    }
    if (nb_samples == nTargetSamples)
    {
        printf("audio buffer is probably too small\n");
    }

    //假设音频的plane只可能是1或者2，对于package audio为1，对于plannar audio为2
    m_nPlanes = av_sample_fmt_is_planar(eTargetFormat) ? 2 : 1;
    m_nAudioSamples = nb_samples;
    m_pAudioLinesize[0] = nb_samples * nTargetChannels * av_get_bytes_per_sample(eTargetFormat);
    m_pAudioLinesize[1] = m_pAudioLinesize[0];
    return SError::ERRORTYPE_SUCCESS;
}

FFMPEG_NAMESPACE_LEAVE