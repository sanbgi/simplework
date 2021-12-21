
#include "av.h"
#include "ffmpeg/CAvFrameConverter.h"
#include "ffmpeg/CAvIn.h"
#include "ffmpeg/CAvOut.h"
#include "sdl/CAvOut_SDLWindow.h"
#include "sdl/CAvOut_SDLSpeaker.h"

class CAvFactory : public CObject, public IAvFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int openAvFileReader(const char* szFileName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoFile(szFileName) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        spPipe.setPtr(spAvIn.getPtr());
        return SError::ERRORTYPE_SUCCESS;
    }

    int openVideoCapture(const char* szDeviceName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoCapture(szDeviceName) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        spPipe.setPtr(spAvIn.getPtr());
        return SError::ERRORTYPE_SUCCESS;
    }

    int openAudioCapture(const char* szDeviceName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initAudioCapture(szDeviceName) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        spPipe.setPtr(spAvIn.getPtr());
        return SError::ERRORTYPE_SUCCESS;
    }

    int openAvFrameConverter(PAvSample targetSample, SPipe& sPipe) {
        return SError::ERRORTYPE_FAILURE;
    }

    int openWindow(const char* szWindowName, int nWidth, int nHeight, SPipe& spPipe) {
        CPointer<sdl::CAvOut_SDLWindow> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initWindow(szWindowName, nWidth, nHeight) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        spPipe.setPtr(spAvOut.getPtr());
        return SError::ERRORTYPE_SUCCESS;
    }

    int openSpeaker(const char* szDeviceName, SPipe& spPipe) {
        CPointer<sdl::CAvOut_SDLSpeaker> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initSpeaker(szDeviceName) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        spPipe.setPtr(spAvOut.getPtr());
        return SError::ERRORTYPE_SUCCESS;
    }

    int openAvFileWriter(const char* szFileName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvOut> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initAvFile(szFileName, 0, nullptr) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
        spPipe.setPtr(spAvOut.getPtr());
        return SError::ERRORTYPE_SUCCESS;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvFactory, SAvFactory::getClassKey())