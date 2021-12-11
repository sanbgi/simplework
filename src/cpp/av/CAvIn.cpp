
#include "av.h"
#include <vector>
#include "CAvFrame.h"
#include "CAvStreaming.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>
}

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;

class CAvIn : public CObject, public IAvIn{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvIn)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    AvIn openVideoFile(const char* szFileName) {
        ObjectWithPtr<CAvIn> wrapAvOut = CObject::createObjectWithPtr<CAvIn>();
        if( wrapAvOut.pObject->initVideoFile(szFileName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)wrapAvOut.pObject);
    }

    AvIn openCapture(const char* szName) {
        static bool g_bInitialized = false;
        if( !g_bInitialized ) {
            avdevice_register_all();
            g_bInitialized = true;
        }
        ObjectWithPtr<CAvIn> wrapAvOut = CObject::createObjectWithPtr<CAvIn>();
        if( wrapAvOut.pObject->initCapture(szName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)wrapAvOut.pObject);
    }
        
    int getStreamingCount() {
        return m_vecStreamings.size();
    }

    AvStreaming getStreaming(int iStreamingIndex) {
        return m_vecStreamings[iStreamingIndex].spObject;
    }
    int getWidth() {
        return 0;
    }

    int getHeight() {
        return 0;
    }

    int getFrame(AvFrame& frame) {
        //
        // 如果上次流成功读取了数据，则还需要继续读取
        //
        if(m_pContinueReadingStreaming) {
            CObject::ObjectWithPtr<CAvStreaming>* pStreaming = m_pContinueReadingStreaming;
            m_pContinueReadingStreaming = nullptr;
            return receiveFrame(frame, pStreaming);
        }

        CAutoFree<AVPacket> avPacket(av_packet_alloc(), av_packet_free);
        if(av_read_frame(m_pFormatCtx, avPacket)>=0) {
            return sendPackageAndReceiveFrame(frame, avPacket);
        }

        return Error::ERRORTYPE_FAILURE;
    }

public:
    int initVideoFile(const char* szFileName) {
        // 释放之前使用资源
        release();

        // 打开视频流
        m_pFormatCtx = avformat_alloc_context();
        if(avformat_open_input(&m_pFormatCtx,szFileName,NULL,NULL)!=0){
            printf("Couldn't open input stream.\n");
            release();
            return Error::ERRORTYPE_FAILURE;
        }
        m_bOpenedFormatCtx = true;

        // 查找视频流中的具体流信息
        if(avformat_find_stream_info(m_pFormatCtx,NULL)<0){
            printf("Couldn't find stream information.\n");
            release();
            return Error::ERRORTYPE_FAILURE; 
        }

        // 初始化所有流参数
        for(int i=0; i<m_pFormatCtx->nb_streams; i++) {
            CObject::ObjectWithPtr<CAvStreaming> spStream = CObject::createObjectWithPtr<CAvStreaming>();
            if( spStream.pObject->init(m_pFormatCtx->streams[i], i) != Error::ERRORTYPE_SUCCESS ) {
                return Error::ERRORTYPE_FAILURE;
            }
            m_vecStreamings.push_back(spStream);
        }
        return Error::ERRORTYPE_SUCCESS;
    }

    int initCapture(const char* szName) {
        // 释放之前使用资源
        release();
        AVInputFormat *ifmt=av_find_input_format("vfwcap");
        if(nullptr == ifmt) {
            return Error::ERRORTYPE_FAILURE;
        }

        // 打开视频流
        m_pFormatCtx = avformat_alloc_context();
        if(avformat_open_input(&m_pFormatCtx,0,ifmt,NULL)!=0){
            printf("Couldn't open input stream.\n");
            release();
            return Error::ERRORTYPE_FAILURE;
        }
        m_bOpenedFormatCtx = true;

        // 查找视频流中的具体流信息
        if(avformat_find_stream_info(m_pFormatCtx,NULL)<0){
            printf("Couldn't find stream information.\n");
            release();
            return Error::ERRORTYPE_FAILURE; 
        }

        // 初始化所有流参数
        for(int i=0; i<m_pFormatCtx->nb_streams; i++) {
            CObject::ObjectWithPtr<CAvStreaming> spStream = CObject::createObjectWithPtr<CAvStreaming>();
            if( spStream.pObject->init(m_pFormatCtx->streams[i], i) != Error::ERRORTYPE_SUCCESS ) {
                return Error::ERRORTYPE_FAILURE;
            }
            m_vecStreamings.push_back(spStream);
        }
        return Error::ERRORTYPE_SUCCESS;
    }

    int sendPackageAndReceiveFrame(AvFrame& frame, AVPacket* pPackage) {

        CObject::ObjectWithPtr<CAvStreaming>* pStreaming = &m_vecStreamings[pPackage->stream_index];

        AVCodecContext* pCodecCtx = pStreaming->pObject->m_pCodecCtx;
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

    int receiveFrame(AvFrame& frame, CObject::ObjectWithPtr<CAvStreaming>* pStreaming) {
        AVCodecContext* pCodecCtx = pStreaming->pObject->m_pCodecCtx;
        CAutoFree<AVFrame> avFrame(av_frame_alloc(), av_frame_free);
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

        CObject::ObjectWithPtr<CAvFrame> spAvFrame = CObject::createObjectWithPtr<CAvFrame>();
        spAvFrame.pObject->m_pAvFrame = avFrame.detach();
        spAvFrame.pObject->m_spAvStream = pStreaming->spObject;
        spAvFrame.pObject->m_pCodecCtx = pCodecCtx;
        spAvFrame.pObject->m_mapCtx = m_mapCtx;
        frame = spAvFrame.spObject;

        //如果读取成功，则下次继续读取
        m_pContinueReadingStreaming = pStreaming;
        return Error::ERRORTYPE_SUCCESS;
    }

public:
    CAvIn() {
        m_bOpenedFormatCtx = false;
        m_pFormatCtx = nullptr;
        m_pContinueReadingStreaming = nullptr;
        m_mapCtx = NamedMap::createMap();
    }
    ~CAvIn() {
        release();
    }
    void release() {
        m_vecStreamings.clear();
        if(m_pFormatCtx) {
            if( m_bOpenedFormatCtx ) {
                avformat_close_input(&m_pFormatCtx);
            }else{
                avformat_free_context(m_pFormatCtx);
            }
             m_pFormatCtx = nullptr;
        }
    }

private:
    bool m_bOpenedFormatCtx;
    AVFormatContext* m_pFormatCtx;
    CObject::ObjectWithPtr<CAvStreaming>* m_pContinueReadingStreaming;
    std::vector<CObject::ObjectWithPtr<CAvStreaming>> m_vecStreamings;
    NamedMap m_mapCtx;
};
SIMPLEWORK_FACTORY_REGISTER(CAvIn, AvIn::getClassKey())