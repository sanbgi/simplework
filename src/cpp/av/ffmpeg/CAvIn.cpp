
#include "av_ffmpeg.h"
#include "CAvIn.h"
#include "CAvFrame.h"
#include "CAvInStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvIn");

int CAvIn::getStreamingSize() {
    return (int)m_arrStreamings.size();
}
const PAvStreaming* CAvIn::getStreamingAt(int iPos) {
    if(iPos < 0 || iPos >= m_arrStreamings.size() ) {
        sCtx.warn("尝试读取不存在的视频流信息");
        return nullptr;
    }
    return &m_arrStreamings.at(iPos);
}

int CAvIn::createAvFileReader(const char* szFileName, SAvIn& spIn) {
    CPointer<ffmpeg::CAvIn> spAvIn;
    CObject::createObject(spAvIn);
    if( spAvIn->initVideoFile(szFileName) != sCtx.success() ) {
        return sCtx.error();
    }
    spIn.setPtr(spAvIn.getPtr());
    return sCtx.success();
}

int CAvIn::initVideoFile(const char* szFileName) {
    // 释放之前使用资源
    release();

    // 打开视频流
    m_spFormatCtx.take(avformat_alloc_context(), [](AVFormatContext* pCtx){avformat_free_context(pCtx);});
    if(avformat_open_input(&m_spFormatCtx,szFileName,NULL,NULL)!=0){
        printf("Couldn't open input stream.\n");
        release();
        return sCtx.error();
    }
    m_spOpenedCtx.take(m_spFormatCtx.untake(), [](AVFormatContext* pCtx){avformat_close_input(&pCtx);});

    // 查找视频流中的具体流信息
    if(avformat_find_stream_info(m_spOpenedCtx,NULL)<0){
        printf("Couldn't find stream information.\n");
        release();
        return sCtx.error(); 
    }

    // 初始化所有流参数
    for(unsigned int i=0; i<m_spOpenedCtx->nb_streams; i++) {
        CPointer<CAvInStreaming> spStreaming;
        CObject::createObject(spStreaming);
        if( spStreaming->init(m_spOpenedCtx->streams[i], i) != sCtx.success() ) {
            return sCtx.error();
        }
        m_arrAvStreamings.push_back(spStreaming);
        m_arrStreamings.push_back(spStreaming->getPAvStreaming());
    }
    return sCtx.success();
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
        return sCtx.error("无法打开输入设备");
    }
    m_spOpenedCtx.take(m_spFormatCtx.untake(), [](AVFormatContext* pCtx){avformat_close_input(&pCtx);});

    // 查找视频流中的具体流信息
    if(avformat_find_stream_info(m_spOpenedCtx,NULL)<0){
        return sCtx.error("读取视频流信息失败"); 
    }

    // 初始化所有流参数
    for(unsigned int i=0; i<m_spOpenedCtx->nb_streams; i++) {
        CPointer<CAvInStreaming> spStreaming;
        CObject::createObject(spStreaming);
        if( spStreaming->init(m_spOpenedCtx->streams[i], i) != sCtx.success() ) {
            return sCtx.error();
        }
        m_arrAvStreamings.push_back(spStreaming);
        m_arrStreamings.push_back(spStreaming->getPAvStreaming());
    }
    return sCtx.success();
}

int CAvIn::readFrame(SAvFrame& spAvFrame) {
    //
    // 如果上次流成功读取了数据，则还需要继续读取
    //
    std::vector<CAvInStreaming*>::reverse_iterator it = m_arrToReadingStreamings.rbegin();
    if(it != m_arrToReadingStreamings.rend() ) {
        CAvInStreaming* pStreaming = *it;
        m_arrToReadingStreamings.pop_back();
        if( receiveFrame(spAvFrame, pStreaming) != sCtx.success() ) {
            if(pStreaming->m_isCompleted) {
                return readFrame(spAvFrame);
            }
            return sCtx.error("读取帧失败");
        }
        return sCtx.success();
    }

    //
    // 如果文件已经读取结束，则直接返回失败，无法读取更多的帧
    //
    if(m_isCompoeted) {
        return sCtx.error();
    }

    CTaker<AVPacket*> spPacket( av_packet_alloc(),
                                [](AVPacket* pPtr){av_packet_free(&pPtr);});

    int retCode = av_read_frame(m_spOpenedCtx, spPacket);
    switch(retCode) {
        case 0:
            return sendPackageAndReceiveFrame(spAvFrame, spPacket);

        case AVERROR_EOF:
            m_isCompoeted = true;
            return sendPackageAndReceiveFrame(spAvFrame, nullptr);

        default:
            return sCtx.error(retCode, "读取帧信息错误");
    }
}

int CAvIn::sendPackageAndReceiveFrame(SAvFrame& spAvFrame, AVPacket* pPackage) {

    //
    // 如果读取帧结束，则刷新所有流解码上下文，并将所有的流加入到待读取流队列中，重新读取
    //
    if(pPackage == nullptr) {
        std::vector<CPointer<CAvInStreaming>>::iterator it = m_arrAvStreamings.begin();
        while(it != m_arrAvStreamings.end()) {
            AVCodecContext* pCodecCtx = (*it)->m_spCodecCtx;
            int ret = avcodec_send_packet(pCodecCtx, pPackage);
            switch(ret) {
                case 0:
                case AVERROR_EOF:
                    break;

                default:
                    return sCtx.error("视频解码上下文无法正确处理视频流结束包"); 
            }
            m_arrToReadingStreamings.push_back((*it));
            it++;
        }
        return readFrame(spAvFrame);
    }

    CAvInStreaming* pStreaming = m_arrAvStreamings[pPackage->stream_index];

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
        case AVERROR_EOF:
            break;

        default:
            return sCtx.error(); 
    }
    return receiveFrame(spAvFrame, pStreaming);
}

int CAvIn::receiveFrame(SAvFrame& spAvFrame, CAvInStreaming* pStreaming) {
    AVCodecContext* pCodecCtx = pStreaming->m_spCodecCtx;
    CTaker<AVFrame*> spFrame(av_frame_alloc(), [](AVFrame* pFrame){
        av_frame_free(&pFrame);
    });
    int ret = avcodec_receive_frame(pCodecCtx, spFrame);
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
        pStreaming->m_isCompleted = true;
        return sCtx.error();

    case AVERROR(EAGAIN):
        return readFrame(spAvFrame);

    default:
        return sCtx.error();
    }

    //如果读取成功，则下次继续读取
    m_arrToReadingStreamings.push_back(pStreaming);
    return CAvFrame::createFrame(pStreaming->m_pAvStream, pStreaming->m_avStreaming.streamingId, spFrame, spAvFrame);
}

bool CAvIn::isCompleted() {
    return m_isCompoeted;
}

CAvIn::CAvIn() {
    m_isCompoeted = false;
}

CAvIn::~CAvIn() {
    release();
}

void CAvIn::release() {
    m_arrAvStreamings.clear();
    m_spOpenedCtx.release();
    m_spFormatCtx.release();
}

FFMPEG_NAMESPACE_LEAVE
