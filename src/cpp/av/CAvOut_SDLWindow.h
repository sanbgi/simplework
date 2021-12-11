
#include "av.h"
#include <SDL2/SDL.h>

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_AV_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

class CAvOut_SDLWindow : public CObject, public IAvOut{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int initWindow(const char* szWindowName, int width, int height) {
        
        release();

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            return Error::ERRORTYPE_FAILURE;

        //创建窗口
        m_pWindow = SDL_CreateWindow("example04: for mediaplayer", 0, 0, width, height, 0);
        if (nullptr == m_pWindow)
            return Error::ERRORTYPE_FAILURE;

        m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
	    if (nullptr == m_pRenderer) {
            return Error::ERRORTYPE_FAILURE;
        }

        m_nWinWidth = width;
        m_nWinHeight = height;
        return Error::ERRORTYPE_SUCCESS;
    }

    int putFrame(const AvFrame& frame) {
        
        if(frame->getFrameType() != AvFrame::AVSTREAMTYPE_VIDEO ) {
            return Error::ERRORTYPE_FAILURE;
        }

        Tensor spTensor = frame->getFrameVideoImage(AvFrame::AVFRAMEIMAGETYPE_RGB);
        const Tensor& spDimTensor = spTensor->getDimVector();
        const int* pDim = spDimTensor->getDataPtr<int>();
        int width = pDim[0];
        int height = pDim[1];
        int depth = pDim[2]*8;
        int pitch = width*pDim[2];

        void *pixels = (void*)spTensor->getDataPtr<unsigned char>();
        /* 
        int rmask, gmask, bmask, amask;
        rmask = 0xFF000000; gmask = 0x00FF0000; bmask = 0x0000FF00; amask = 0x000000FF;	// RGBA8888模式
        //rmask = 0xFF000000; gmask = 0x00FF0000; bmask = 0x0000FF00; amask = 0x00000000;	// RGB8888模式
        //创建一个RGB Surface

        CAutoPointer<SDL_Surface> pTmpSurface(
            SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask), 
            SDL_FreeSurface );
        if (nullptr == pTmpSurface) {
            return Error::ERRORTYPE_FAILURE;
        }

        //创建Texture
        CAutoPointer<SDL_Texture> pTexture(SDL_CreateTextureFromSurface(pRenderer, pTmpSurface), SDL_DestroyTexture);
        if (nullptr == pTexture) {
            return Error::ERRORTYPE_FAILURE;
        }*/
        SDL_Renderer* pRenderer = m_pRenderer;
        CAutoPointer<SDL_Texture> pTexture(SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height), SDL_DestroyTexture);
        if (nullptr == pTexture) {
            return Error::ERRORTYPE_FAILURE;
        }

        SDL_Rect srcRect, dstRect;
        dstRect.x = srcRect.x = 0;
        dstRect.y = srcRect.y = 0;
        srcRect.w = width;
        srcRect.h = height;
        dstRect.w = m_nWinWidth;
        dstRect.h = m_nWinHeight;

        SDL_UpdateTexture(pTexture, &srcRect, pixels, pitch);

        //清除Renderer
        SDL_RenderClear(pRenderer);
        //Texture复制到Renderer
        SDL_RenderCopy(pRenderer, pTexture, &srcRect, &dstRect);
        //更新Renderer显示
        SDL_RenderPresent(pRenderer);
        
        return Error::ERRORTYPE_SUCCESS;
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
