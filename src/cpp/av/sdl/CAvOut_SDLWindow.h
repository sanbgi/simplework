#ifndef __SimpleWork_av_sdl_CAvOut_SDLWindow_h__
#define __SimpleWork_av_sdl_CAvOut_SDLWindow_h__

#include "av_sdl.h"
#include "CAvSampleType.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SDL_NAMESPACE_ENTER

class CAvOut_SDLWindow : public CObject, public IAvOut{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    static SCtx sCtx;

public://IAvOut
    int writeFrame(const SAvFrame& spFrame) {
        if(!spFrame) {
            return close();
        }

        SAvFrame spOut;
        if( m_spConverter->pipeIn(spFrame, spOut) != sCtx.success() ) {
            return sCtx.error("转化图片格式失败");
        }

        const PAvFrame* pFrame = spOut->getFramePtr();
        PAvSample sampleMeta = pFrame->sampleMeta;
        SDL_PixelFormatEnum ePixelFormat = CAvSampleType::toPixelFormat(sampleMeta.sampleFormat);
        int width = sampleMeta.videoWidth;
        int height = sampleMeta.videoHeight;
        SDL_Renderer* pRenderer = m_pRenderer;
        CTaker<SDL_Texture*> spTexture(
                                SDL_CreateTexture(pRenderer, ePixelFormat, SDL_TEXTUREACCESS_STREAMING, width, height),
                                SDL_DestroyTexture
                            );
        if (!spTexture) {
            return sCtx.error();
        }

        SDL_Rect srcRect, dstRect;
        dstRect.x = srcRect.x = 0;
        dstRect.y = srcRect.y = 0;
        srcRect.w = width;
        srcRect.h = height;
        dstRect.w = m_nWinWidth;
        dstRect.h = m_nWinHeight;
        void *pixels = pFrame->ppPlanes[0];
        int pitch = pFrame->pPlaneLineSizes[0];
        SDL_UpdateTexture(spTexture, &srcRect, pixels, pitch);

        //清除Renderer
        SDL_RenderClear(pRenderer);
        //Texture复制到Renderer
        SDL_RenderCopy(pRenderer, spTexture, &srcRect, &dstRect);
        //更新Renderer显示
        SDL_RenderPresent(pRenderer);
        
        return sCtx.success();
    }

public:
    static int createWindow(const char* szWindowName, int nWidth, int nHeight, SAvOut& spAvOutWindow) {
        CPointer<sdl::CAvOut_SDLWindow> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initWindow(szWindowName, nWidth, nHeight) != sCtx.success() ) {
            return sCtx.error();
        }
        spAvOutWindow.setPtr(spAvOut.getPtr());
        return sCtx.success();
    }

    int initWindow(const char* szWindowName, int nWidth, int nHeight) {
        
        release();

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            return sCtx.error();

        //创建窗口
        m_pWindow = SDL_CreateWindow("SimpleWork: for mediaplayer", 0, 0, nWidth, nHeight, 0);
        if (nullptr == m_pWindow)
            return sCtx.error();

        m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
	    if (nullptr == m_pRenderer) {
            return sCtx.error();
        }

        m_nWinWidth = nWidth;
        m_nWinHeight = nHeight;
        PAvSample sampleMeta;
        sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
        sampleMeta.sampleFormat = EAvSampleFormat::AvSampleFormat_Video_RGB;
        sampleMeta.videoWidth = nWidth;
        sampleMeta.videoHeight = nHeight;
        if( SAvFactory::getAvFactory()->openAvFrameConverter(sampleMeta, m_spConverter) != sCtx.success() ) {
            return sCtx.error();
        }
        return sCtx.success();
    }

    int close() {
        release();
        return sCtx.success();
    }

public:
    CAvOut_SDLWindow() {
        m_pWindow = nullptr;
        m_pRenderer = nullptr;
    }
    ~CAvOut_SDLWindow() {
        release();
    }    

    void release() {
        if(m_pRenderer) {
            SDL_DestroyRenderer(m_pRenderer);
            m_pRenderer = nullptr;
        }
        if(m_pWindow) {
            SDL_DestroyWindow(m_pWindow);
            m_pWindow = nullptr;
            SDL_Quit();
        }
    }

private:
    SAvNetwork m_spConverter;
    SDL_Window* m_pWindow;
    SDL_Renderer* m_pRenderer;
    int m_nWinWidth;
    int m_nWinHeight;
};
SCtx CAvOut_SDLWindow::sCtx("CAvOut_SDLWindow");

SDL_NAMESPACE_LEAVE

#endif//__SimpleWork_av_sdl_CAvOut_SDLWindow_h__