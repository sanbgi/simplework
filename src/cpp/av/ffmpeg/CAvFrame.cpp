
#include "av_ffmpeg.h"
#include "CAvFrame.h"
#include "CAvOutStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvFrame");

AVFrame* CAvFrame::allocAvFramePtr(AVStream* pStreaming, int iStringingId) {
    m_spAvFrame.take(av_frame_alloc(), [](AVFrame* pFrame){
        av_frame_free(&pFrame);
    });
    AVFrame* pAvFrame = m_spAvFrame;
    AVCodecParameters * pCodecCtx = pStreaming->codecpar;
    switch (pCodecCtx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        {
            m_avFrame.sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
            m_avFrame.sampleMeta.sampleFormat = CAvSampleType::convert((AVPixelFormat)pCodecCtx->format);
            m_avFrame.sampleMeta.videoWidth = pCodecCtx->width;
            m_avFrame.sampleMeta.videoHeight = pCodecCtx->height;
        }
        break;
    
    case AVMEDIA_TYPE_AUDIO:
        {
            m_avFrame.sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio;
            m_avFrame.sampleMeta.sampleFormat = CAvSampleType::convert((AVSampleFormat)pCodecCtx->format);
            m_avFrame.sampleMeta.audioRate = pCodecCtx->sample_rate;
            m_avFrame.sampleMeta.audioChannels = pCodecCtx->channels;
        }
        break;

    default:
        sCtx.error("未知的帧类型");
        return nullptr;
    }
    AVRational r = pStreaming->time_base;
    m_avFrame.timeRate = r.den/r.num;
    m_avFrame.streamingId = iStringingId;
    return pAvFrame;
}

int CAvFrame::setAVFrameToPAvFrame() {
    if(!m_spAvFrame) {
        return sCtx.error("没有帧信息，无法更新");
    }

    AVFrame* pAvFrame = m_spAvFrame;
    PAvFrame& avFrame = m_avFrame;
    
    avFrame.ppPlanes = pAvFrame->data;
    avFrame.pPlaneLineSizes = pAvFrame->linesize;

    //
    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据，这里面要注意，是否存在
    //  planar audio，并且通道数超过8？如果存在这种情况，则这里的数据是存在丢失的
    //
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pAvFrame->data[i]; i++ ) {
        nPlanes = i+1;
    }

    avFrame.nPlanes = nPlanes;
    avFrame.timeStamp = pAvFrame->pts;
    switch(m_avFrame.sampleMeta.sampleType) {
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
        {
            return sCtx.error("暂时没有支持的帧类型");
        }
    }
    return sCtx.success();
}

const PAvFrame* CAvFrame::getFramePtr() {

    return &m_avFrame;
}


//
// 从ffmpeg中拷贝过来的代码，目录在：lavfutils.c
//
int ff_load_image(uint8_t *data[4], int linesize[4],
                  int *w, int *h, enum AVPixelFormat *pix_fmt,
                  const char *filename, void *log_ctx)
{
    AVInputFormat *iformat = NULL;
    AVFormatContext *format_ctx = NULL;
    const AVCodec *codec;
    AVCodecContext *codec_ctx = NULL;
    AVCodecParameters *par;
    AVFrame *frame = NULL;
    int ret = 0;
    AVPacket pkt;
    AVDictionary *opt=NULL;

    iformat = av_find_input_format("image2pipe");
    if ((ret = avformat_open_input(&format_ctx, filename, iformat, NULL)) < 0) {
        av_log(log_ctx, AV_LOG_ERROR,
               "Failed to open input file '%s'\n", filename);
        return ret;
    }

    if ((ret = avformat_find_stream_info(format_ctx, NULL)) < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Find stream info failed\n");
        goto end;
    }

    par = format_ctx->streams[0]->codecpar;
    codec = avcodec_find_decoder(par->codec_id);
    if (!codec) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to find codec\n");
        ret = AVERROR(EINVAL);
        goto end;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to alloc video decoder context\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = avcodec_parameters_to_context(codec_ctx, par);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to copy codec parameters to decoder context\n");
        goto end;
    }

    av_dict_set(&opt, "thread_type", "slice", 0);
    if ((ret = avcodec_open2(codec_ctx, codec, &opt)) < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to open codec\n");
        goto end;
    }

    if (!(frame = av_frame_alloc()) ) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to alloc frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = av_read_frame(format_ctx, &pkt);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to read frame from file\n");
        goto end;
    }

    ret = avcodec_send_packet(codec_ctx, &pkt);
    av_packet_unref(&pkt);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Error submitting a packet to decoder\n");
        goto end;
    }

    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to decode image from file\n");
        goto end;
    }

    *w       = frame->width;
    *h       = frame->height;
    *pix_fmt = (AVPixelFormat)frame->format;

    if ((ret = av_image_alloc(data, linesize, *w, *h, *pix_fmt, 16)) < 0)
        goto end;
    ret = 0;

    av_image_copy(data, linesize, (const uint8_t **)frame->data, frame->linesize, *pix_fmt, *w, *h);

end:
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    av_frame_free(&frame);
    av_dict_free(&opt);

    if (ret < 0)
        av_log(log_ctx, AV_LOG_ERROR, "Error loading image file '%s'\n", filename);
    return ret;
}


int CAvFrame::loadImage(const char* szFileName, SAvFrame& spFrame) {
    CPointer<CAvFrame> sp;
    CObject::createObject(sp);
    sp->m_spAvFrame.take(av_frame_alloc(), [](AVFrame* pFrame){
        av_frame_free(&pFrame);
    });
    AVFrame* pAvFrame = sp->m_spAvFrame;
    AVPixelFormat pixForamt;
    if( ff_load_image(
                pAvFrame->data, pAvFrame->linesize, 
                &pAvFrame->width, &pAvFrame->height, 
                (AVPixelFormat*)&pAvFrame->format, szFileName, nullptr) < 0 ) {
        return sCtx.error("读取图片文件失败");
    }
    
    PAvFrame* pFrame = &sp->m_avFrame;
    pFrame->nHeight = pAvFrame->height;
    pFrame->nWidth = pAvFrame->width;
    pFrame->streamingId = 0;
    pFrame->timeRate = 0;
    pFrame->timeStamp = 0;
    pFrame->sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
    pFrame->sampleMeta.sampleFormat = CAvSampleType::convert(pixForamt);
    pFrame->sampleMeta.videoHeight = pAvFrame->height;
    pFrame->sampleMeta.videoWidth = pAvFrame->width;
    pFrame->pPlaneLineSizes = pAvFrame->linesize;
    pFrame->ppPlanes = pAvFrame->data;
    pFrame->nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pAvFrame->data[i]; i++ ) {
        pFrame->nPlanes = i+1;
    }
    spFrame.setPtr(sp.getPtr());
    return sCtx.success();;
}

int CAvFrame::saveImage(const char* szFileName, const SAvFrame& spFrame) {
    
    const PAvFrame* pFrame = spFrame ? spFrame->getFramePtr() : nullptr;
    if(pFrame == nullptr) {
        return sCtx.error("图片中无法写入无效的帧数据");
    }
    ((PAvFrame*)pFrame)->timeStamp = 0;

    // 分配AVFormatContext对象
    AVFormatContext* pFormatContext = nullptr;
    if( avformat_alloc_output_context2(&pFormatContext, nullptr, "image2", szFileName) < 0 ) {
        return sCtx.error("写入上下文创建失败");
    }
    CTaker<AVFormatContext*> spFormatCtx(pFormatContext, [](AVFormatContext* pCtx){
        avformat_free_context(pCtx);
    });

    
    // 创建流
    PAvStreaming avStreaming;
    avStreaming.streamingId = 0;
    avStreaming.timeRate = 25;
    avStreaming.timeDuration = 0;
    avStreaming.frameMeta = pFrame->sampleMeta;
    CPointer<CAvOutStreaming> spStreaming;
    CObject::createObject(spStreaming);
    if(spStreaming->init(pFormatContext, &avStreaming) != sCtx.success()) {
        return sCtx.error("初始化视频流失败");
    }

    if( spStreaming->open(pFormatContext) != sCtx.success() ) {
        return sCtx.error("打开图片对应的流失败");
    }

    av_dump_format(pFormatContext, 0, szFileName, 1);


    // 创建并初始化一个和该url相关的AVIOContext
    if( avio_open(&pFormatContext->pb, szFileName, AVIO_FLAG_WRITE) < 0){
        return sCtx.error("无法打开输出文件");
    }
    CTaker<AVIOContext*> spIOContext(pFormatContext->pb, [](AVIOContext* pCtx) {
        avio_close(pCtx);
    });

    //写入头部信息
    if( avformat_write_header(pFormatContext, NULL) < 0 ) {
        return sCtx.error("写入图片文件头信息失败");
    };
 
    //写入图片信息
    if( spStreaming->writeFrame(pFormatContext, spFrame) != sCtx.success() ) {
        return sCtx.error("向图片文件写入帧数据失败");
    }

    //写入尾部信息
    if( av_write_trailer(pFormatContext) < 0 ) {
        return sCtx.error("写入图片文件尾部信息失败");
    };
 
    return sCtx.success();
}


FFMPEG_NAMESPACE_LEAVE