
#include "CAvFilter.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

int CAvFilter::putFrame(const PAvFrame* pSrc, PAvFrame::FVisitor visitor) {
    switch(m_targetSample.sampleType) {
        case EAvSampleType::AvSampleType_Audio:
            return convertAudio(AV_SAMPLE_FMT_NONE, pSrc, visitor);

        case EAvSampleType::AvSampleType_Video:
            return convertVideo(AV_PIX_FMT_NONE, pSrc, visitor);
    }
    return SError::ERRORTYPE_FAILURE;
}

int CAvFilter::createFilter(const PAvSample& targetSample, SAvFilter& spFilter) {

    SObject spAvFilter;
    CAvFilter* pAvFilter = CObject::createObject<CAvFilter>(spAvFilter);
    if( pAvFilter->initFilter(targetSample) != SError::ERRORTYPE_SUCCESS) {
        return SError::ERRORTYPE_FAILURE;
    }
    spFilter.setPtr((IAvFilter*)pAvFilter);
    return SError::ERRORTYPE_SUCCESS;
}

int CAvFilter::initFilter(const PAvSample& targetSample) {
    CFormat fmt;
    switch(targetSample.sampleType) {
        case EAvSampleType::AvSampleType_Video:
            fmt = CFormat(targetSample.videoWidth, targetSample.videoHeight, CAvSampleType::toPixFormat(targetSample.sampleFormat) );
            break;

        case EAvSampleType::AvSampleType_Audio:
            fmt = CFormat(targetSample.audioRate, targetSample.audioChannels, CAvSampleType::toSampleFormat(targetSample.sampleFormat) );
            break;

        default:
            return SError::ERRORTYPE_FAILURE;
    }
    m_targetFormat = fmt;
    m_targetSample = targetSample;
    return SError::ERRORTYPE_SUCCESS;
}

CAvFilter::CAvFilter(){

    //表示缓存还未初始化
    m_pVideoData[0] = nullptr;
    m_ppAudioData = nullptr;
}

CAvFilter::~CAvFilter() {
    releaseVideoCtx();
    releaseAudioCtx();
}

void CAvFilter::releaseVideoCtx() {
    releaseVideoData();
    m_spSwsContext.release();
}

void CAvFilter::releaseAudioCtx() {
    releaseAudioData();
    m_spSwrCtx.release();
}

void CAvFilter::releaseVideoData() {
    if(m_pVideoData[0] != nullptr) {
        av_freep(&m_pVideoData[0]);
        m_pVideoData[0] = nullptr;
    }
}

void CAvFilter::releaseAudioData() {
    if(m_ppAudioData) {
        av_freep(m_ppAudioData);
        m_ppAudioData = nullptr;
    }
}

int CAvFilter::convertVideo(AVPixelFormat sourceFormat, const PAvFrame* pSrc, PAvFrame::FVisitor visitor) {
    const PAvSample& srcMeta = pSrc->sampleMeta;

    AVPixelFormat targetFormat = (AVPixelFormat)m_targetFormat.m_nFormat;
    int targetWidth = m_targetFormat.m_nWidth;
    int targetHeight = m_targetFormat.m_nHeight;

    if(sourceFormat == AV_PIX_FMT_NONE)
        sourceFormat = CAvSampleType::toPixFormat(srcMeta.sampleFormat);
    int sourceWidth = srcMeta.videoWidth;
    int sourceHeight = srcMeta.videoHeight;

    //
    //  如果格式相同，则不要转化了，直接用
    //
    if(sourceFormat == targetFormat && sourceWidth == targetWidth && sourceHeight == targetHeight) {
        return visitor->visit(pSrc);
    }

    //
    // 如果上次数据源格式与当前数据源格式不同，则需要重新创建转化器
    //
    if(m_spSwsContext) {
        if( m_lastSourceFormat != CFormat(sourceWidth, sourceHeight, sourceFormat) ){
            releaseVideoCtx();
        }
    }

    //
    // 没有转化器时，创建转化器
    //
    if(!m_spSwsContext) {
        //
        // 创建转化器
        //
        SwsContext* pSwsContext = sws_getContext(
            //源图像的 宽 , 高 , 图像像素格式
            sourceWidth, sourceHeight, sourceFormat,
            //目标图像 大小不变 , 不进行缩放操作 , 只将像素格式设置成 RGBA 格式的
            targetWidth, targetHeight, targetFormat,
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
            targetWidth, targetHeight, targetFormat, 1) < 0 ){
            releaseVideoCtx();
            return SError::ERRORTYPE_FAILURE;
        }

        m_lastSourceFormat = CFormat(sourceWidth, sourceHeight, sourceFormat);
    }

    int ret_height = sws_scale(
        //SwsContext *swsContext 转换上下文
        m_spSwsContext,
        //要转换的数据内容
        pSrc->planeDatas,
        //数据中每行的字节长度
        pSrc->planeLineSizes,
        0,
        sourceHeight,
        //转换后目标图像数据存放在这里
        m_pVideoData,
        //转换后的目标图像行数
        m_pVideoLinesizes
    );

    // 如果转化结果尺寸与想要的目标尺寸不一致，则转化失败
    if( ret_height != m_targetFormat.m_nHeight ) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据
    for( int i=0; i<AV_NUM_DATA_POINTERS && m_pVideoLinesizes[i]; i++ ) {
        m_nPlanes = i;
    }

    PAvFrame targetFrame = *pSrc;
    targetFrame.sampleMeta = m_targetSample;
    targetFrame.samplePlanes = m_nPlanes;
    targetFrame.planeDatas = m_pVideoData;
    targetFrame.planeLineSizes = m_pVideoLinesizes;
    return visitor->visit(&targetFrame);
}

int CAvFilter::convertAudio(AVSampleFormat sourceFormat, const PAvFrame* pSrc, PAvFrame::FVisitor visitor) {
    const PAvSample& srcMeta = pSrc->sampleMeta;

    AVSampleFormat targetFormat = (AVSampleFormat)m_targetFormat.m_nFormat;
    int targetChannels = m_targetFormat.m_nChannels;
    int targetRate = m_targetFormat.m_nRate;

    if(sourceFormat == AV_SAMPLE_FMT_NONE)
        sourceFormat = CAvSampleType::toSampleFormat(srcMeta.sampleFormat);
    int sourceRate = srcMeta.audioRate;
    int sourceChannels = srcMeta.audioChannels;

    //
    //  如果格式相同，则不要转化了，直接用
    //
    if(sourceFormat == targetFormat && sourceChannels == targetChannels && sourceRate == targetRate) {
        return visitor->visit(pSrc);
    }

    //
    //  如果上次使用的格式转化器不能用了，则释放它，后面会重新创建，这里
    //  的作用时在一系列相同转化下，转化器无需反复创建
    //
    if( m_spSwrCtx ) {
        if( m_lastSourceFormat != CFormat(sourceRate, sourceChannels, sourceFormat) ){
            releaseAudioCtx();
        }
    }

    if( !m_spSwrCtx  ) {
        int64_t targetLayout = av_get_default_channel_layout(targetChannels);
        int64_t sourceLayout = av_get_default_channel_layout(sourceChannels);
        m_spSwrCtx.take(swr_alloc_set_opts(
                                    NULL,
                                    targetLayout, 
                                    targetFormat, 
                                    targetRate,
                                    sourceLayout,           
                                    sourceFormat, 
                                    sourceRate,
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
        m_lastSourceFormat = CFormat(sourceRate, sourceChannels, sourceFormat);
    }

    releaseAudioData();

    int nBufSize;
    int nTargetSamples = (int64_t)pSrc->samples * targetRate / sourceRate + 256;
    if( nBufSize = av_samples_alloc_array_and_samples( &m_ppAudioData, nullptr,
                        targetChannels, nTargetSamples, 
                        targetFormat, 0) <0 ){
        releaseVideoCtx();
        return SError::ERRORTYPE_FAILURE;
    }
    
    //
    // 这里面关系比较复杂，extended_data与pData格式相同，一般情况下也是相同的，其含义是
    //  对于planar audio，都是指向指针数组的指针，其中数组中每一个指针指向一个channel的数据
    //  对于package audio，同样是指向指针数组额指针，不过，指针数组中，只有第一个指针有效，指向具体数据
    //
    const uint8_t **in = (const uint8_t **)pSrc->planeDatas;

    // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
    int nb_samples = swr_convert(m_spSwrCtx, m_ppAudioData, nTargetSamples, in, pSrc->samples);
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
    m_nPlanes = av_sample_fmt_is_planar(targetFormat) ? 2 : 1;
    m_nAudioSamples = nb_samples;
    m_pAudioLinesize[0] = nb_samples * targetChannels * av_get_bytes_per_sample(targetFormat);
    m_pAudioLinesize[1] = m_pAudioLinesize[0];

    PAvFrame targetFrame = *pSrc;
    targetFrame.sampleMeta = m_targetSample;
    targetFrame.samplePlanes = m_nPlanes;
    targetFrame.planeDatas = m_ppAudioData;
    targetFrame.planeLineSizes = m_pAudioLinesize;
    return visitor->visit(&targetFrame);
}

FFMPEG_NAMESPACE_LEAVE
