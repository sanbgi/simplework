
#include "CAvFrameConverter.h"
#include "CAvSampleType.h"
#include "CAvFrame.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvFrameConverter");

int CAvFrameConverter::pipeIn(const SAvFrame& spIn, SAvFrame& spOut) {
    //空帧不需要做任何转化
    if(!spIn) {
        return sCtx.success();
    }

    const PAvFrame* pAvFrame = spIn->getFramePtr();
    switch(m_targetSample.sampleType) {
        case EAvSampleType::AvSampleType_Audio:
            return convertAudio(spIn, spOut);

        case EAvSampleType::AvSampleType_Video:
            return convertVideo(spIn, spOut);
    }
    return sCtx.error();
}

int CAvFrameConverter::createFilter(const PAvSample& targetSample, SAvNetwork& spFilter) {
    CPointer<CAvFrameConverter> spAvFrameConverter;
    CObject::createObject(spAvFrameConverter);
    if( spAvFrameConverter->initFilter(targetSample) != sCtx.success()) {
        return sCtx.error();
    }
    spFilter.setPtr(spAvFrameConverter.getPtr());
    return sCtx.success();
}

int CAvFrameConverter::initFilter(const PAvSample& targetSample) {
    CFormat fmt;
    switch(targetSample.sampleType) {
        case EAvSampleType::AvSampleType_Video:
            fmt = CFormat(targetSample.videoWidth, targetSample.videoHeight, CAvSampleType::toPixFormat(targetSample.sampleFormat) );
            break;

        case EAvSampleType::AvSampleType_Audio:
            fmt = CFormat(targetSample.audioRate, targetSample.audioChannels, CAvSampleType::toSampleFormat(targetSample.sampleFormat) );
            break;

        default:
            return sCtx.error();
    }
    m_targetFormat = fmt;
    m_targetSample = targetSample;
    return sCtx.success();
}

CAvFrameConverter::CAvFrameConverter(){
}

CAvFrameConverter::~CAvFrameConverter() {
    releaseVideoCtx();
    releaseAudioCtx();
}

void CAvFrameConverter::releaseVideoCtx() {
    m_spSwsContext.release();
}

void CAvFrameConverter::releaseAudioCtx() {
    m_spSwrCtx.release();
}


int CAvFrameConverter::convertVideo(const SAvFrame& spIn, SAvFrame& spOut) {
    
    const PAvFrame* pSrc = spIn->getFramePtr();
    const PAvSample& srcMeta = pSrc->sampleMeta;

    AVPixelFormat targetFormat = (AVPixelFormat)m_targetFormat.m_nFormat;
    int targetWidth = m_targetFormat.m_nWidth;
    int targetHeight = m_targetFormat.m_nHeight;

    AVPixelFormat sourceFormat = CAvSampleType::toPixFormat(srcMeta.sampleFormat);
    int sourceWidth = srcMeta.videoWidth;
    int sourceHeight = srcMeta.videoHeight;

    //
    //  如果格式相同，则不要转化了，直接用
    //
    if(sourceFormat == targetFormat && sourceWidth == targetWidth && sourceHeight == targetHeight) {
        spOut = spIn;
        return sCtx.success();
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
        switch (sourceFormat) {
        case AV_PIX_FMT_YUVJ420P :
            sourceFormat = AV_PIX_FMT_YUV420P;
            break;
        case AV_PIX_FMT_YUVJ422P  :
            sourceFormat = AV_PIX_FMT_YUV422P;
            break;
        case AV_PIX_FMT_YUVJ444P   :
            sourceFormat = AV_PIX_FMT_YUV444P;
            break;
        case AV_PIX_FMT_YUVJ440P :
            sourceFormat = AV_PIX_FMT_YUV440P;
            break;
        }

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
            return sCtx.error("图像转化器创建失败");
        }

        m_spSwsContext.take(pSwsContext, sws_freeContext);
        m_lastSourceFormat = CFormat(sourceWidth, sourceHeight, sourceFormat);
    }

    CPointer<CAvFrame> spAvFrameOut;
    CObject::createObject(spAvFrameOut);
     
    if( CAvFrame::allocImageDataBuffer(spAvFrameOut->m_spBuffer, 
                targetFormat, targetWidth, targetHeight, 
                spAvFrameOut->m_pLinesizes,spAvFrameOut->m_ppPlanes) != sCtx.success() ) {
        return sCtx.error("分配图像内存失败");
    }

    int ret_height = sws_scale(
        //SwsContext *swsContext 转换上下文
        m_spSwsContext,
        //要转换的数据内容
        pSrc->ppPlanes,
        //数据中每行的字节长度
        pSrc->pPlaneLineSizes,
        0,
        sourceHeight,
        //转换后目标图像数据存放在这里
        spAvFrameOut->m_ppPlanes,
        //转换后的目标图像行数
        spAvFrameOut->m_pLinesizes
    );

    // 如果转化结果尺寸与想要的目标尺寸不一致，则转化失败
    if( ret_height != m_targetFormat.m_nHeight ) {
        return sCtx.error("转化出来的图像的尺寸与想要的尺寸不一致，转化失败");
    }

    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && spAvFrameOut->m_ppPlanes[i]; i++ ) {
        nPlanes = i+1;
    }

    PAvFrame& targetFrame = spAvFrameOut->m_avFrame;
    targetFrame = *pSrc;
    targetFrame.sampleMeta = m_targetSample;
    targetFrame.nHeight = m_targetSample.videoHeight;
    targetFrame.nWidth = m_targetSample.videoWidth;
    targetFrame.nPlanes = nPlanes;
    targetFrame.ppPlanes = spAvFrameOut->m_ppPlanes;
    targetFrame.pPlaneLineSizes = spAvFrameOut->m_pLinesizes;
    spOut.setPtr(spAvFrameOut.getPtr());
    return sCtx.success();
}

int CAvFrameConverter::convertAudio(const SAvFrame& spIn, SAvFrame& spOut) {
        
    const PAvFrame* pSrc = spIn->getFramePtr();
    const PAvSample& srcMeta = pSrc->sampleMeta;

    AVSampleFormat targetFormat = (AVSampleFormat)m_targetFormat.m_nFormat;
    int targetChannels = m_targetFormat.m_nChannels;
    int targetRate = m_targetFormat.m_nRate;

    AVSampleFormat sourceFormat = CAvSampleType::toSampleFormat(srcMeta.sampleFormat);
    int sourceRate = srcMeta.audioRate;
    int sourceChannels = srcMeta.audioChannels;

    //
    //  如果格式相同，则不要转化了，直接用
    //
    if(sourceFormat == targetFormat && sourceChannels == targetChannels && sourceRate == targetRate) {
        spOut = spIn;
        return sCtx.success();
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
            return sCtx.error();
        }

        if( swr_init(m_spSwrCtx) < 0 ) {
            releaseAudioCtx();
            return sCtx.error();
        }
        m_lastSourceFormat = CFormat(sourceRate, sourceChannels, sourceFormat);
    }

    CPointer<CAvFrame> spAvFrameOut;
    CObject::createObject(spAvFrameOut);
    int nTargetSamples = swr_get_out_samples(m_spSwrCtx,pSrc->nWidth);
    if(nTargetSamples < 1) {
        return sCtx.error("计算目标缓冲区大小异常");
    }
    if( CAvFrame::allocAudioSampleDataBuffer(spAvFrameOut->m_spBuffer, 
                targetFormat, targetChannels, nTargetSamples, 
                spAvFrameOut->m_pLinesizes,spAvFrameOut->m_ppPlanes) != sCtx.success() ) {
        return sCtx.error("分配音频帧内存失败");
    }

    //
    // 这里面关系比较复杂，extended_data与pData格式相同，一般情况下也是相同的，其含义是
    //  对于planar audio，都是指向指针数组的指针，其中数组中每一个指针指向一个channel的数据
    //  对于package audio，同样是指向指针数组额指针，不过，指针数组中，只有第一个指针有效，指向具体数据
    //
    const uint8_t **in = (const uint8_t **)pSrc->ppPlanes;

    // 音频重采样：返回值是重采样后得到的音频数据中单个声道的样本数
    int nb_samples = swr_convert(m_spSwrCtx, spAvFrameOut->m_ppPlanes, nTargetSamples, in, pSrc->nWidth);
    if (nb_samples < 0) {
        return sCtx.error("swr_convert()失败");
    }

    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && spAvFrameOut->m_ppPlanes[i]; i++ ) {
        nPlanes = i+1;
    }
    //TODO: 如何计算？
    spAvFrameOut->m_pLinesizes[0] = nb_samples * targetChannels * av_get_bytes_per_sample(targetFormat);
    spAvFrameOut->m_pLinesizes[1] = spAvFrameOut->m_pLinesizes[0];

    PAvFrame& targetFrame = spAvFrameOut->m_avFrame;
    targetFrame.sampleMeta = m_targetSample;
    targetFrame.nWidth = nb_samples;
    targetFrame.nHeight = 1;
    targetFrame.nPlanes = nPlanes;
    targetFrame.ppPlanes = spAvFrameOut->m_ppPlanes;
    targetFrame.pPlaneLineSizes = spAvFrameOut->m_pLinesizes;
    spOut.setPtr(spAvFrameOut.getPtr());
    return sCtx.success();
}

FFMPEG_NAMESPACE_LEAVE
