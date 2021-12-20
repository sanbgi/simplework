
#include "av_ffmpeg.h"
#include "CAvOut.h"
#include "CAvOutStreaming.h"

FFMPEG_NAMESPACE_ENTER

int CAvOut::initAvFile(const char* szFileName, int nStreamings, PAvStreaming* pStreamings) {
    //
    // 格式上下文
    //
    AVFormatContext* pFormatContext = nullptr;
    if( avformat_alloc_output_context2(&pFormatContext, nullptr, nullptr, szFileName) < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_spFormatContext.take(pFormatContext,avformat_free_context);

    //
    // 创建视频流
    //
    for(int i=0; i<nStreamings; i++) {
        //
        // 创建并初始化流对象
        //
        PAvStreaming* pStreaming = pStreamings+i;
        CPointer<CAvOutStreaming> spStreaming;
        CObject::createObject(spStreaming);
        if(spStreaming->init(pFormatContext, pStreaming) != SError::ERRORTYPE_SUCCESS) {
            return SError::ERRORTYPE_FAILURE;
        }
        m_arrStreamings.push_back(spStreaming);
    }

    //
    // 打开所有视频流
    //
    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end()) {
        if( (*it)->open(pFormatContext) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        it++;
    }

    //
    // 打开视频流
    //
    if( avio_open(&pFormatContext->pb, szFileName, AVIO_FLAG_WRITE) < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_spIOContext.take(pFormatContext->pb, [](AVIOContext* pPtr){
        avio_close(pPtr);
    });

    //
    // 写入头部信息
    //
    if( avformat_write_header(pFormatContext, NULL) < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    return SError::ERRORTYPE_SUCCESS;
}

int CAvOut::pushFrame(const PAvFrame* pFrame) {
    //如果写入一个空的帧，则表示要关闭写文件
    if(pFrame == nullptr) {
        close();
        return SError::ERRORTYPE_FAILURE;
    }

    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end() ) {
        if((*it)->pushFrame(m_spFormatContext, pFrame) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        it++;
    }

    return SError::ERRORTYPE_SUCCESS;
}

int CAvOut::close() {
    int err = av_write_trailer(m_spFormatContext);
    if( err < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end()) {
        if( (*it)->close(m_spFormatContext) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        it++;
    }

    AVIOContext* pIo = m_spIOContext.untake();
    if( avio_close(pIo) < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    return SError::ERRORTYPE_SUCCESS;
}

CAvOut::CAvOut() {
}

CAvOut::~CAvOut() {
    release();
}

void CAvOut::release() {
}


FFMPEG_NAMESPACE_LEAVE
