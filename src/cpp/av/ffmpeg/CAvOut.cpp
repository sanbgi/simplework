
#include "av_ffmpeg.h"
#include "CAvOut.h"
#include "CAvOutStreaming.h"

FFMPEG_NAMESPACE_ENTER
static SCtx sCtx("CAvOut");
int CAvOut::pushData(const PData& rData, IVisitor<const PData&>* pReceiver) {  
    int idType =rData.getType();
    if(idType == SData::getStructTypeIdentifier<PAvStreaming>() ) {
        const PAvStreaming* pStreaming = CData<PAvStreaming>(rData);
        CPointer<CAvOutStreaming> spStreaming;
        CObject::createObject(spStreaming);
        if(spStreaming->init(m_spFormatContext, pStreaming) != sCtx.Success()) {
            return sCtx.Error();
        }
        m_arrStreamings.push_back(spStreaming);
        return sCtx.Success();
    }else if(idType == SData::getStructTypeIdentifier<PAvFrame>() ) {
        return pushFrame(CData<PAvFrame>(rData));
    }
    return sCtx.Success();
}

int CAvOut::initAvFile(const char* szFileName, int nStreamings, PAvStreaming* pStreamings) {
    //
    // 格式上下文
    //
    AVFormatContext* pFormatContext = nullptr;
    if( avformat_alloc_output_context2(&pFormatContext, nullptr, nullptr, szFileName) < 0 ) {
        return sCtx.Error();
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
        if(spStreaming->init(pFormatContext, pStreaming) != sCtx.Success()) {
            return sCtx.Error();
        }
        m_arrStreamings.push_back(spStreaming);
    }
    m_strFileName = szFileName;
    return sCtx.Success();
}

int CAvOut::pushFrame(const PAvFrame* pFrame) {
    if( !m_bOpened ) {
        //
        // 打开所有视频流
        //
        std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
        while(it != m_arrStreamings.end()) {
            if( (*it)->open(m_spFormatContext) != sCtx.Success() ) {
                return sCtx.Error();
            }
            it++;
        }

        //
        // 打开视频流
        //
        if( avio_open(&m_spFormatContext->pb, m_strFileName.c_str(), AVIO_FLAG_WRITE) < 0 ) {
            return sCtx.Error();
        }
        m_spIOContext.take(m_spFormatContext->pb, [](AVIOContext* pPtr){
            avio_close(pPtr);
        });

        //
        // 写入头部信息
        //
        if( avformat_write_header(m_spFormatContext, NULL) < 0 ) {
            return sCtx.Error();
        }

        m_bOpened = true;
    }

    //如果写入一个空的帧，则表示要关闭写文件
    if(pFrame == nullptr) {
        close();
        return sCtx.Error();
    }

    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end() ) {
        if((*it)->pushFrame(m_spFormatContext, pFrame) != sCtx.Success() ) {
            return sCtx.Error();
        }
        it++;
    }

    return sCtx.Success();
}

int CAvOut::close() {
    int err = av_write_trailer(m_spFormatContext);
    if( err < 0 ) {
        return sCtx.Error();
    }

    std::vector<CPointer<CAvOutStreaming>>::iterator it = m_arrStreamings.begin();
    while(it != m_arrStreamings.end()) {
        if( (*it)->close(m_spFormatContext) != sCtx.Success() ) {
            return sCtx.Error();
        }
        it++;
    }

    AVIOContext* pIo = m_spIOContext.untake();
    if( avio_close(pIo) < 0 ) {
        return sCtx.Error();
    }

    return sCtx.Success();
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
