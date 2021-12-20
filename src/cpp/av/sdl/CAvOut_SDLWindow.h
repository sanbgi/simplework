#ifndef __SimpleWork_av_sdl_CAvOut_SDLWindow_h__
#define __SimpleWork_av_sdl_CAvOut_SDLWindow_h__

#include "av_sdl.h"
#include "CAvSampleType.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SDL_NAMESPACE_ENTER

class CAvOut_SDLWindow : public CObject, public IAvOut, IVisitor<const PAvFrame*>{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int initWindow(const char* szWindowName, PAvSample& sampleMeta) {
        
        release();

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            return SError::ERRORTYPE_FAILURE;

        //创建窗口
        m_pWindow = SDL_CreateWindow("SimpleWork: for mediaplayer", 0, 0, sampleMeta.videoWidth, sampleMeta.videoHeight, 0);
        if (nullptr == m_pWindow)
            return SError::ERRORTYPE_FAILURE;

        m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
	    if (nullptr == m_pRenderer) {
            return SError::ERRORTYPE_FAILURE;
        }

        m_nWinWidth = sampleMeta.videoWidth;
        m_nWinHeight = sampleMeta.videoHeight;
        switch (sampleMeta.sampleFormat)
        {
        case EAvSampleFormat::AvSampleFormat_Video_RGB:
        case EAvSampleFormat::AvSampleFormat_Video_RGBA:
            break;
        
        default:
            sampleMeta.sampleFormat = EAvSampleFormat::AvSampleFormat_Video_RGB;
            break;
        }
        if( SAvFilter::createFilter(sampleMeta, m_spFilter) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        return SError::ERRORTYPE_SUCCESS;
    }


    int pushFrame(const PAvFrame* pFrame) {
        if(pFrame == nullptr) {
            return close();
        }

        return m_spFilter->putFrame(pFrame, this);
    }

    int visit(const PAvFrame* pFrame) {

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
            return SError::ERRORTYPE_FAILURE;
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
        
        return SError::ERRORTYPE_SUCCESS;
    }

    int close() {
        release();
        return SError::ERRORTYPE_SUCCESS;
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
    SAvFilter m_spFilter;
    SDL_Window* m_pWindow;
    SDL_Renderer* m_pRenderer;
    int m_nWinWidth;
    int m_nWinHeight;
};

SDL_NAMESPACE_LEAVE

#endif//__SimpleWork_av_sdl_CAvOut_SDLWindow_h__