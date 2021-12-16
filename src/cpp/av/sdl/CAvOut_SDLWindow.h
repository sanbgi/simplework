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

public:
    int initWindow(const char* szWindowName, SAvSampleMeta& sampleMeta) {
        
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
        return SError::ERRORTYPE_SUCCESS;
    }

    int putVariable(const char* szKey, const char* szValue) {
        return SError::ERRORTYPE_FAILURE;
    }

    int writeFrame(const SAvFrame& frame) {
        STensor spTensor = frame->getData();
        SAvSampleMeta sampleMeta = frame->getStreaming()->getSampleMeta();

        const STensor& spDimTensor = spTensor->getDimVector();
        const int* pDim = spDimTensor->getDataPtr<int>();
        int height = pDim[0];
        int width = pDim[1];
        int depth = pDim[2]*8;
        int pitch = width*pDim[2];

        SDL_PixelFormatEnum ePixelFormat = CAvSampleType::toPixelFormat(sampleMeta.sampleType);
        void *pixels = (void*)spTensor->getDataPtr<unsigned char>();
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
    SDL_Window* m_pWindow;
    SDL_Renderer* m_pRenderer;
    int m_nWinWidth;
    int m_nWinHeight;
};

SDL_NAMESPACE_LEAVE

#endif//__SimpleWork_av_sdl_CAvOut_SDLWindow_h__