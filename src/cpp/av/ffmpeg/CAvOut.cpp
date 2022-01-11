
#include "av_ffmpeg.h"
#include "CAvOut.h"
#include "CAvSampleType.h"
#include "CAvOutStreaming.h"

FFMPEG_NAMESPACE_ENTER
static SCtx sCtx("CAvOut");

int CAvOut::createAvFile(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings, SAvOut& spAvOut) {
    CPointer<ffmpeg::CAvOut> spOut;
    CObject::createObject(spOut);
    if( spOut->initAvFile(szFileName, nStreamings, pStreamings) != sCtx.success() ) {
        return sCtx.error();
    }
    spAvOut.setPtr(spOut.getPtr());
    return sCtx.success();
}

int CAvOut::initAvFile(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings) {
    //
    // 格式上下文
    //
    AVFormatContext* pFormatContext = nullptr;
    if( avformat_alloc_output_context2(&pFormatContext, nullptr, nullptr, szFileName) < 0 ) {
        return sCtx.error();
    }
    m_spFormatContext.take(pFormatContext,avformat_free_context);

    //
    // 创建视频流
    //
    for(int i=0; i<nStreamings; i++) {
        //
        // 创建并初始化流对象
        //
        const PAvStreaming* pStreaming = pStreamings+i;
        CPointer<CAvOutStreaming> spStreaming;
        CObject::createObject(spStreaming);
        if(spStreaming->init(pFormatContext, pStreaming) != sCtx.success()) {
            return sCtx.error();
        }
        m_arrStreamings.push_back(spStreaming);
    }

    //
    // 打开所有视频流
    //
    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end()) {
        if( (*it)->open(m_spFormatContext) != sCtx.success() ) {
            return sCtx.error("打开单个视频流失败");
        }
        it++;
    }

    //
    // 打开视频流
    //
    if( avio_open(&m_spFormatContext->pb, szFileName, AVIO_FLAG_WRITE) < 0 ) {
        return sCtx.error("打开整个视频的音视频流失败");
    }
    m_spIOContext.take(m_spFormatContext->pb, [](AVIOContext* pPtr){
        avio_close(pPtr);
    });

    //
    // 写入头部信息
    //
    if( avformat_write_header(m_spFormatContext, NULL) < 0 ) {
        return sCtx.error();
    }

    m_bOpened = true;
    m_strFileName = szFileName;
    return sCtx.success();
}

int CAvOut::writeFrame(const SAvFrame& avFrame) {
    const PAvFrame* pFrame = avFrame ? avFrame->getFramePtr() : nullptr;
    //如果写入一个空的帧，则表示要关闭写文件
    if(pFrame == nullptr) {
        return close();
    }

    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end() ) {
        if( (*it)->m_iStreamingId == pFrame->streamingId ) {
            if((*it)->writeFrame(m_spFormatContext, avFrame) != sCtx.success() ) {
                return sCtx.error("视频帧写入失败");
            }
            break;
        }
        it++;
    }

    return sCtx.success();    
}

int CAvOut::close() {
    int err = av_write_trailer(m_spFormatContext);
    if( err < 0 ) {
        return sCtx.error();
    }

    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end()) {
        if( (*it)->close(m_spFormatContext) != sCtx.success() ) {
            return sCtx.error();
        }
        it++;
    }

    AVIOContext* pIo = m_spIOContext.untake();
    if( avio_close(pIo) < 0 ) {
        return sCtx.error();
    }

    return sCtx.success();
}

CAvOut::CAvOut() {
    m_bOpened = false;
}

CAvOut::~CAvOut() {
    release();
}

void CAvOut::release() {
}


FFMPEG_NAMESPACE_LEAVE
