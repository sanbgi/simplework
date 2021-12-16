
#include "av_ffmpeg.h"
#include "CAvIn.h"
#include "CAvStreaming.h"
#include "CAvFrame.h"

FFMPEG_NAMESPACE_ENTER

int CAvIn::getStreaming(SAvStreaming& rStreaming) {
    if( m_vecAvStreamings.size() == 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    if(!rStreaming) {
        rStreaming = m_vecAvStreamings[0];
    }else{
        int iStreaming = rStreaming->getStreamingId() + 1;
        if( iStreaming >= m_vecAvStreamings.size() ) {
            return SError::ERRORTYPE_FAILURE;
        }
        rStreaming = m_vecAvStreamings[iStreaming];
    }
    return SError::ERRORTYPE_SUCCESS;
}

int CAvIn::changeStreamingSampleMeta(int iStreamingId, const CAvSampleMeta& sampleMeta) {
    if(iStreamingId < 0 || iStreamingId >= m_vecCAvStreamings.size() ) {
        return SError::ERRORTYPE_FAILURE;
    }
    return m_vecCAvStreamings[iStreamingId]->setSampleMeta(sampleMeta);
}


int CAvIn::readFrame(SAvFrame& frame) {
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
    if(av_read_frame(m_spOpenedCtx, spPacket)>=0) {
        return sendPackageAndReceiveFrame(frame, spPacket);
    }

    return SError::ERRORTYPE_FAILURE;
}

int CAvIn::initVideoFile(const char* szFileName) {
    // 释放之前使用资源
    release();

    // 打开视频流
    m_spFormatCtx.take(avformat_alloc_context(), [](AVFormatContext* pCtx){avformat_free_context(pCtx);});
    if(avformat_open_input(&m_spFormatCtx,szFileName,NULL,NULL)!=0){
        printf("Couldn't open input stream.\n");
        release();
        return SError::ERRORTYPE_FAILURE;
    }
    m_spOpenedCtx.take(m_spFormatCtx.untake(), [](AVFormatContext* pCtx){avformat_close_input(&pCtx);});

    // 查找视频流中的具体流信息
    if(avformat_find_stream_info(m_spOpenedCtx,NULL)<0){
        printf("Couldn't find stream information.\n");
        release();
        return SError::ERRORTYPE_FAILURE; 
    }

    // 初始化所有流参数
    for(int i=0; i<m_spOpenedCtx->nb_streams; i++) {
        SObject spObject;
        CAvStreaming* pCAvStreaming = CObject::createObject<CAvStreaming>(spObject);
        if( pCAvStreaming->init(m_spOpenedCtx->streams[i], i) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        m_vecCAvStreamings.push_back(pCAvStreaming);
        m_vecAvStreamings.push_back(spObject);
    }
    return SError::ERRORTYPE_SUCCESS;
}

void CAvIn::initDeviceRegistry() {
    static bool g_bInitialized = false;
    if( !g_bInitialized ) {
        avdevice_register_all();
        g_bInitialized = true;
    }
}


int CAvIn::initVideoCapture(const char* szName) {
    release();
    initDeviceRegistry();
    AVInputFormat *ifmt = av_find_input_format("dshow");
    /*
    AVInputFormat *ifmt;
    if(szName != nullptr)
        ifmt = av_find_input_format(szName);
    else
        ifmt = av_input_video_device_next(nullptr);
    */
    return initCapture(ifmt, szName);
}

int CAvIn::initAudioCapture(const char* szName) {
    release();
    initDeviceRegistry();
    AVInputFormat *ifmt = av_find_input_format("dshow");
    /*
    AVInputFormat *ifmt;
    if(szName != nullptr)
        ifmt = av_find_input_format(szName);
    else
        ifmt = av_input_audio_device_next(nullptr);
    */
    return initCapture(ifmt, szName);
}

int CAvIn::initCapture(AVInputFormat* pInputForamt, const char* szName) {

    // 打开视频流
    m_spFormatCtx.take(avformat_alloc_context(), [](AVFormatContext* pCtx){avformat_free_context(pCtx);});
    if(avformat_open_input(&m_spFormatCtx,szName,pInputForamt,NULL)!=0){
        printf("Couldn't open input stream.\n");
        release();
        return SError::ERRORTYPE_FAILURE;
    }
    m_spOpenedCtx.take(m_spFormatCtx.untake(), [](AVFormatContext* pCtx){avformat_close_input(&pCtx);});

    // 查找视频流中的具体流信息
    if(avformat_find_stream_info(m_spOpenedCtx,NULL)<0){
        printf("Couldn't find stream information.\n");
        release();
        return SError::ERRORTYPE_FAILURE; 
    }

    // 初始化所有流参数
    for(int i=0; i<m_spOpenedCtx->nb_streams; i++) {
        SObject spObject;
        CAvStreaming* pCAvStreaming = CObject::createObject<CAvStreaming>(spObject);
        if( pCAvStreaming->init(m_spOpenedCtx->streams[i], i) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        m_vecCAvStreamings.push_back(pCAvStreaming);
        m_vecAvStreamings.push_back(spObject);
    }
    return SError::ERRORTYPE_SUCCESS;
}

int CAvIn::sendPackageAndReceiveFrame(SAvFrame& frame, AVPacket* pPackage) {

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
            if( ret == SError::ERRORTYPE_SUCCESS ) {
                if( (ret = avcodec_send_packet(pCodecCtx, pPackage)) != 0 ) {
                    //按理说，receiveFrame后，应该可以重新发送Package，什么原因造成不能?
                    return SError::ERRORTYPE_FAILURE;
                }
            }
            return ret;

        default:
            return SError::ERRORTYPE_FAILURE; 
    }
    return receiveFrame(frame, pStreaming);
}

int CAvIn::receiveFrame(SAvFrame& frame, CAvStreaming* pStreaming) {
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
        return SError::ERRORTYPE_FAILURE;

    case AVERROR(EAGAIN):
        return readFrame(frame);

    default:
        return SError::ERRORTYPE_FAILURE;
    }

    //如果读取成功，则下次继续读取
    m_pContinueReadingStreaming = pStreaming;
    return CAvFrame::createAvFrame(avFrame, pStreaming, frame);
}

CAvIn::CAvIn() {
    m_pContinueReadingStreaming = nullptr;
}

CAvIn::~CAvIn() {
    release();
}

void CAvIn::release() {
    m_vecAvStreamings.clear();
    m_vecCAvStreamings.clear();
    m_spOpenedCtx.release();
    m_spFormatCtx.release();
}

FFMPEG_NAMESPACE_LEAVE
