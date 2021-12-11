#include <string>
#include "Av.h"
#include "CAvFrame.h"
#include "CSwsCtx.h"

AvStreaming::AvStreamingType CAvFrame::getStreamingType() {
    return m_spAvStream->getStreamingType();
}

AvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}

Tensor CAvFrame::getVideoImage(AvFrame::AvFrameImageType eType) {
    std::string strPixFormatName;
    AVPixelFormat ePixFormat;
    switch(eType) {
    case AvFrame::AVFRAMEIMAGETYPE_RGB:
        ePixFormat = AV_PIX_FMT_RGB24;
        strPixFormatName = "AVFRAMEIMAGETYPE_RGB";
        break;

    case AvFrame::AVFRAMEIMAGETYPE_RGBA:
        ePixFormat = AV_PIX_FMT_RGBA;
        strPixFormatName = "AVFRAMEIMAGETYPE_RGBA";
        break;

    default:
        return Tensor();
    }

    SwsCtx spCtx = m_mapCtx->getAt(strPixFormatName.c_str());
    if( !spCtx ) {
        Object spSwsObject = CObject::createObject<CSwsCtx>();
        spCtx = spSwsObject;
        if( spCtx->init(m_pCodecCtx, ePixFormat) != Error::ERRORTYPE_SUCCESS ) {
            return Tensor();
        }
        m_mapCtx->putAt(strPixFormatName.c_str(), spSwsObject);
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
    m_eAvFrameType = AvStreaming::AVSTREAMTYPE_UNKNOWN;
}

CAvFrame::~CAvFrame() {
    attachAvFrame(nullptr);
}

