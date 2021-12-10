#include "Av.h"
#include "CAvFrame.h"

using namespace sw::math;

SIMPLEWORK_INTERFACECLASS_ENTER0(SwsCtx)
    
    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.ISwsMap", 211210)

        virtual int init(AVCodecContext* pCodecCtx) = 0;

        virtual Tensor convertImage(AVFrame* pAvFrame) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(SwsCtx)

class CSwsCtx : public CObject, ISwsCtx {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ISwsCtx)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int init(AVCodecContext* pCodecCtx) {

        release();

        m_pSwsContext = sws_getContext(
            //源图像的 宽 , 高 , 图像像素格式
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            //目标图像 大小不变 , 不进行缩放操作 , 只将像素格式设置成 RGBA 格式的
            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA,
            //使用的转换算法 , FFMPEG 提供了许多转换算法 , 有快速的 , 有高质量的 , 需要自己测试
            SWS_BILINEAR,
            //源图像滤镜 , 这里传 NULL 即可
            0,
            //目标图像滤镜 , 这里传 NULL 即可
            0,
            //额外参数 , 这里传 NULL 即可
            0
        );
        if(m_pSwsContext == nullptr) {
            return Error::ERRORTYPE_FAILURE;
        }

        if( av_image_alloc(m_pImagePointers, m_pLinesizes,
            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA, 1) < 0 ){
            release();
            return Error::ERRORTYPE_FAILURE;
        }

        return Error::ERRORTYPE_SUCCESS;
    }

    Tensor convertImage(AVFrame* pAvFrame) {
        if( m_pSwsContext == nullptr ) {
            return Tensor();
        }

        sws_scale(
            //SwsContext *swsContext 转换上下文
            m_pSwsContext,
            //要转换的数据内容
            pAvFrame->data,
            //数据中每行的字节长度
            pAvFrame->linesize,
            0,
            pAvFrame->height,
            //转换后目标图像数据存放在这里
            m_pImagePointers,
            //转换后的目标图像行数
            m_pLinesizes
        );

        int nplanesize = pAvFrame->width*pAvFrame->height;
        int dimsize[3] = { pAvFrame->width, pAvFrame->height, 4 };
        Tensor spDimTensor = Tensor::createVector(3, dimsize);
        Tensor spImage = Tensor::createTensor(spDimTensor, nplanesize*4, m_pImagePointers[0]);
        return spImage;
    }

    CSwsCtx() {
        m_pImagePointers[0] = nullptr;
        m_pSwsContext = nullptr;
    }

    ~CSwsCtx() {
        if(m_pImagePointers[0] != nullptr) {
            av_freep(&m_pImagePointers[0]);
            m_pImagePointers[0] = nullptr;
        }
    }
    void release() {
        if(m_pImagePointers[0] != nullptr) {
            av_freep(&m_pImagePointers[0]);
            m_pImagePointers[0] = nullptr;
        }

        if(m_pSwsContext) {
            sws_freeContext(m_pSwsContext);
            m_pSwsContext = nullptr;
        }
    }

private:
    SwsContext* m_pSwsContext;
    uint8_t *m_pImagePointers[4];
    int m_pLinesizes[4];
};


AvStreaming::AvStreamingType CAvFrame::getStreamingType() {
    return m_spAvStream->getStreamingType();
}

AvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}

Tensor CAvFrame::getVideoImage() { 
    SwsCtx spCtx = m_mapCtx->getAt("sws");
    if( !spCtx ) {
        Object spSwsObject = CObject::createObject<CSwsCtx>();
        spCtx = spSwsObject;
        if( spCtx->init(m_pCodecCtx) != Error::ERRORTYPE_SUCCESS ) {
            return Tensor();
        }
        m_mapCtx->putAt("sws", spSwsObject);
    }
    return spCtx->convertImage(m_pAvFrame);
}

void CAvFrame::attachAvFrame(AVFrame* pAvFrame) {
    if(m_pAvFrame) {
        av_frame_free(&m_pAvFrame);
        m_pAvFrame = nullptr;
    }
    m_pAvFrame = pAvFrame;
}

CAvFrame::CAvFrame() {
    m_pAvFrame = nullptr;
    m_eAvFrameType = AvFrame::AVFRAMETYPE_UNKNOWN;
}

CAvFrame::~CAvFrame() {
    attachAvFrame(nullptr);
}

