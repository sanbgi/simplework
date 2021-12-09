
#include "av.h"
#include <vector>
#include "CAvFrame.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <SDL2/SDL.h>
}

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;

template<typename Q> class CAutoFree {
public:
    typedef void (*FUN)(Q**);
    CAutoFree(Q* pQ, FUN fun) {
        m_ptr = pQ;
        m_fun = fun;
    }
    ~CAutoFree() {
        if(m_ptr) {
            (*m_fun)(&m_ptr);
        }
    }
    Q* operator->() {
        return m_ptr;
    }
    operator Q*() {
        return m_ptr;
    }
    Q* detach() {
        Q* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

private:
    Q* m_ptr;
    FUN m_fun;
};

class CAvIn : public CObject, public IAvIn{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvIn)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
        
    int init(const char* szFileName) {
        release();

        m_pFormatCtx = avformat_alloc_context();
        if(avformat_open_input(&m_pFormatCtx,szFileName,NULL,NULL)!=0){
            printf("Couldn't open input stream.\n");
            release();
            return Error::ERRORTYPE_FAILURE;
        }
        m_bOpenedFormatCtx = true;
        if(avformat_find_stream_info(m_pFormatCtx,NULL)<0){
            printf("Couldn't find stream information.\n");
            release();
            return Error::ERRORTYPE_FAILURE; 
        }

        for(int i=0; i<m_pFormatCtx->nb_streams; i++) {
            AVMediaType codeType = m_pFormatCtx->streams[i]->codecpar->codec_type;
            switch(codeType) {
                case AVMediaType::AVMEDIA_TYPE_VIDEO:
                case AVMediaType::AVMEDIA_TYPE_AUDIO:
                    {
                        AVCodecParameters* pCodecParameter = m_pFormatCtx->streams[i]->codecpar;
                        if(pCodecParameter == nullptr) {
                            release();
                            return Error::ERRORTYPE_FAILURE;
                        }

                        CAutoFree<AVCodecContext> pCodecCtx(avcodec_alloc_context3(nullptr), avcodec_free_context);
                        if( avcodec_parameters_to_context(pCodecCtx, pCodecParameter) < 0 ) {
                            release();
                            return Error::ERRORTYPE_FAILURE;
                        }

                        AVCodec* pCodec=avcodec_find_decoder(pCodecParameter->codec_id);
                        if(pCodec==NULL){
                            printf("Codec not found.\n");
                            release();
                            return Error::ERRORTYPE_FAILURE;
                        }

                        if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
                            printf("Could not open codec.\n");
                            release();
                            return Error::ERRORTYPE_FAILURE;
                        }
                        m_vecCodecCtxs.push_back(pCodecCtx.detach());
                    }
                    break;

                default:
                    m_vecCodecCtxs.push_back(nullptr);
                    break;
            }
        }

        return Error::ERRORTYPE_SUCCESS;
    }

    int getWidth() {
        return 0;
    }

    int getHeight() {
        return 0;
    }

    int getFrame(AvFrame& frame) {
        if(m_pContinueReadingCtx) {
            AVCodecContext* pCodecCtx = m_pContinueReadingCtx;
            m_pContinueReadingCtx = nullptr;
            return receiveFrame(frame, pCodecCtx);
        }

        CAutoFree<AVPacket> avPacket(av_packet_alloc(), av_packet_free);
        if(av_read_frame(m_pFormatCtx, avPacket)>=0) {
            return sendPackageAndReceiveFrame(frame, avPacket);
        }

        return Error::ERRORTYPE_FAILURE;
    }


public:
    int sendPackageAndReceiveFrame(AvFrame& frame, AVPacket* pPackage) {

        AVCodecContext* pCodecCtx = m_vecCodecCtxs[pPackage->stream_index];
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
                ret = receiveFrame(frame, pCodecCtx);
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
        return receiveFrame(frame, pCodecCtx);
    }

    int receiveFrame(AvFrame& frame,  AVCodecContext* pCodecCtx) {

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
        spAvFrame.pObject->attachAvFrame(avFrame.detach());
        switch(pCodecCtx->codec_type) {
        case AVMediaType::AVMEDIA_TYPE_VIDEO:
            spAvFrame.pObject->m_eAvFrameType = AvFrame::AVFRAMETYPE_VIDEO;
            break;
        case AVMediaType::AVMEDIA_TYPE_AUDIO:
            spAvFrame.pObject->m_eAvFrameType = AvFrame::AVFRAMETYPE_AUDIO;
            break;
        }
        frame = spAvFrame.spObject; 

        //如果读取成功，则下次继续读取
        m_pContinueReadingCtx = pCodecCtx;
        return Error::ERRORTYPE_SUCCESS;
    }

public:
    CAvIn() {
        m_bOpenedFormatCtx = false;
        m_pFormatCtx = nullptr;
        m_pContinueReadingCtx = nullptr;
    }
    ~CAvIn() {
        release();
    }
    void release() {

        for(std::vector<AVCodecContext*>::iterator it=m_vecCodecCtxs.begin(); it!=m_vecCodecCtxs.end(); it++) {
            AVCodecContext* pCodecCtx = *it;
            if(pCodecCtx) {
                avcodec_free_context(&pCodecCtx);
            }
        }
        m_vecCodecCtxs.clear();

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
    AVCodecContext* m_pContinueReadingCtx;
    std::vector<AVCodecContext*> m_vecCodecCtxs;
};
SIMPLEWORK_FACTORY_REGISTER(CAvIn, "sw.av.AvIn")