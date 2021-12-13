
#include "av_ffmpeg.h"
#include "CAvIn.h"
#include "CAudioFrame.h"
#include "CVideoFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

int CAvIn::getStreamingCount() {
    return m_vecAvStreamings.size();
}

AvStreaming CAvIn::getStreaming(int iStreamingIndex) {
    return m_vecAvStreamings[iStreamingIndex];
}

int CAvIn::getWidth() {
    return 0;
}

int CAvIn::getHeight() {
    return 0;
}

int CAvIn::getFrame(AvFrame& frame) {
    //
    // 如果上次流成功读取了数据，则还需要继续读取
    //
    if(m_pContinueReadingStreaming) {
        CAvStreaming* pStreaming = m_pContinueReadingStreaming;
        m_pContinueReadingStreaming = nullptr;
        return receiveFrame(frame, pStreaming);
    }

    CTaker<AVPacket*> spPacket( av_packet_alloc(),
                                [](AVPacket* pPtr){av_packet_free(&pPtr);});
    if(av_read_frame(m_spFormatCtx, spPacket)>=0) {
        return sendPackageAndReceiveFrame(frame, spPacket);
    }

    return Error::ERRORTYPE_FAILURE;
}

int CAvIn::initVideoFile(const char* szFileName) {
    // 释放之前使用资源
    release();

    // 打开视频流
    m_spFormatCtx.take(avformat_alloc_context(), [](AVFormatContext* pCtx){avformat_free_context(pCtx);});
    if(avformat_open_input(&m_spFormatCtx,szFileName,NULL,NULL)!=0){
        printf("Couldn't open input stream.\n");
        release();
        return Error::ERRORTYPE_FAILURE;
    }
    m_bOpenedFormatCtx = true;

    // 查找视频流中的具体流信息
    if(avformat_find_stream_info(m_spFormatCtx,NULL)<0){
        printf("Couldn't find stream information.\n");
        release();
        return Error::ERRORTYPE_FAILURE; 
    }

    // 初始化所有流参数
    for(int i=0; i<m_spFormatCtx->nb_streams; i++) {
        Object spObject;
        CAvStreaming* pCAvStreaming = CObject::createObject<CAvStreaming>(spObject);
        if( pCAvStreaming->init(m_spFormatCtx->streams[i], i) != Error::ERRORTYPE_SUCCESS ) {
            return Error::ERRORTYPE_FAILURE;
        }
        m_vecCAvStreamings.push_back(pCAvStreaming);
        m_vecAvStreamings.push_back(spObject);
    }
    return Error::ERRORTYPE_SUCCESS;
}

int CAvIn::initVideoCapture(const char* szName) {
    static bool g_bInitialized = false;
    if( !g_bInitialized ) {
        avdevice_register_all();
        g_bInitialized = true;
    }
    
    // 释放之前使用资源
    release();
    AVInputFormat *ifmt=av_find_input_format("vfwcap");
    if(nullptr == ifmt) {
        return Error::ERRORTYPE_FAILURE;
    }

    // 打开视频流
    m_spFormatCtx.take(avformat_alloc_context(), [](AVFormatContext* pCtx){avformat_free_context(pCtx);});
    if(avformat_open_input(&m_spFormatCtx,0,ifmt,NULL)!=0){
        printf("Couldn't open input stream.\n");
        release();
        return Error::ERRORTYPE_FAILURE;
    }
    m_bOpenedFormatCtx = true;

    // 查找视频流中的具体流信息
    if(avformat_find_stream_info(m_spFormatCtx,NULL)<0){
        printf("Couldn't find stream information.\n");
        release();
        return Error::ERRORTYPE_FAILURE; 
    }

    // 初始化所有流参数
    for(int i=0; i<m_spFormatCtx->nb_streams; i++) {
        Object spObject;
        CAvStreaming* pCAvStreaming = CObject::createObject<CAvStreaming>(spObject);
        if( pCAvStreaming->init(m_spFormatCtx->streams[i], i) != Error::ERRORTYPE_SUCCESS ) {
            return Error::ERRORTYPE_FAILURE;
        }
        m_vecCAvStreamings.push_back(pCAvStreaming);
        m_vecAvStreamings.push_back(spObject);
    }
    return Error::ERRORTYPE_SUCCESS;
}

int CAvIn::sendPackageAndReceiveFrame(AvFrame& frame, AVPacket* pPackage) {

    CAvStreaming* pStreaming = m_vecCAvStreamings[pPackage->stream_index];

    AVCodecContext* pCodecCtx = pStreaming->m_spCodecCtx;
    int ret = avcodec_send_packet(pCodecCtx, pPackage);
/*
* @return 0 on success, otherwise negative error code:
*      AVERROR(EAGAIN):   input is not accepted in the current state - user
*                         must read output with avcodec_receive_frame() (once
*                         all output is read, the packet should be resent, and
*                         the call will not fail with EAGAIN).
*      AVERROR_EOF:       the decoder has been flushed, and no new packets can
*                         be sent to it (also returned if more than 1 flush
*                         packet is sent)
*      AVERROR(EINVAL):   codec not opened, it is an encoder, or requires flush
*      AVERROR(ENOMEM):   failed to add packet to internal queue, or similar
*      other errors: legitimate decoding errors
*/
    switch(ret) {
        case 0:
            break;

        case AVERROR(EAGAIN):
            ret = receiveFrame(frame, pStreaming);
            if( ret == Error::ERRORTYPE_SUCCESS ) {
                if( (ret = avcodec_send_packet(pCodecCtx, pPackage)) != 0 ) {
                    //按理说，receiveFrame后，应该可以重新发送Package，什么原因造成不能?
                    return Error::ERRORTYPE_FAILURE;
                }
            }
            return ret;

        default:
            return Error::ERRORTYPE_FAILURE; 
    }
    return receiveFrame(frame, pStreaming);
}

int CAvIn::receiveFrame(AvFrame& frame, CAvStreaming* pStreaming) {
    AVCodecContext* pCodecCtx = pStreaming->m_spCodecCtx;
    CTaker<AVFrame*> avFrame(av_frame_alloc(), [](AVFrame* pFrame){av_frame_free(&pFrame);});
    int ret = avcodec_receive_frame(pCodecCtx, avFrame);
/*
*      0:                 success, a frame was returned
*      AVERROR(EAGAIN):   output is not available in this state - user must try
*                         to send new input
*      AVERROR_EOF:       the decoder has been fully flushed, and there will be
*                         no more output frames
*      AVERROR(EINVAL):   codec not opened, or it is an encoder
*      AVERROR_INPUT_CHANGED:   current decoded frame has changed parameters
*                               with respect to first decoded frame. Applicable
*                               when flag AV_CODEC_FLAG_DROPCHANGED is set.
*      other negative values: legitimate decoding errors
*/     
    switch(ret) {
    case 0:
        break;

    case AVERROR_EOF:
        return Error::ERRORTYPE_FAILURE;

    case AVERROR(EAGAIN):
        return getFrame(frame);

    default:
        return Error::ERRORTYPE_FAILURE;
    }

    //如果读取成功，则下次继续读取
    m_pContinueReadingStreaming = pStreaming;
    return CAvFrame::createAvFrame(avFrame, pStreaming, frame);
}

CAvIn::CAvIn() {
    m_bOpenedFormatCtx = false;
    m_pContinueReadingStreaming = nullptr;
}

CAvIn::~CAvIn() {
    release();
}

void CAvIn::release() {
    m_vecAvStreamings.clear();
    m_vecCAvStreamings.clear();
    if( m_bOpenedFormatCtx ) {
        avformat_close_input(&m_spFormatCtx);
    }
}

FFMPEG_NAMESPACE_LEAVE
