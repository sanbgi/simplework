
#include "av.h"
#include "ffmpeg/CAvFrameConverter.h"
#include "ffmpeg/CAvIn.h"
#include "ffmpeg/CAvOut.h"
#include "sdl/CAvOut_SDLWindow.h"
#include "sdl/CAvOut_SDLSpeaker.h"
#include "pipe/CPipe.h"

static SCtx sCtx("CAvFactory");
class CAvFactory : public CObject, public IAvFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int openAvFileReader(const char* szFileName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoFile(szFileName) != sCtx.Success() ) {
            return sCtx.Error();
        }
        spPipe.setPtr(spAvIn.getPtr());
        return sCtx.Success();
    }

    int openVideoCapture(const char* szDeviceName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoCapture(szDeviceName) != sCtx.Success() ) {
            return sCtx.Error();
        }
        spPipe.setPtr(spAvIn.getPtr());
        return sCtx.Success();
    }

    int openAudioCapture(const char* szDeviceName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initAudioCapture(szDeviceName) != sCtx.Success() ) {
            return sCtx.Error();
        }
        spPipe.setPtr(spAvIn.getPtr());
        return sCtx.Success();
    }

    int openAvFrameConverter(PAvSample targetSample, SPipe& sPipe) {
        return ffmpeg::CAvFrameConverter::createFilter(targetSample, sPipe);
    }

    int openWindow(const char* szWindowName, int nWidth, int nHeight, SPipe& spPipe) {
        CPointer<sdl::CAvOut_SDLWindow> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initWindow(szWindowName, nWidth, nHeight) != sCtx.Success() ) {
            return sCtx.Error();
        }
        spPipe.setPtr(spAvOut.getPtr());
        return sCtx.Success();
    }

    int openSpeaker(const char* szDeviceName, SPipe& spPipe) {
        CPointer<sdl::CAvOut_SDLSpeaker> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initSpeaker(szDeviceName) != sCtx.Success() ) {
            return sCtx.Error();
        }
        spPipe.setPtr(spAvOut.getPtr());
        return sCtx.Success();
    }

    int openAvFileWriter(const char* szFileName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvOut> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initAvFile(szFileName, 0, nullptr) != sCtx.Success() ) {
            return sCtx.Error();
        }
        spPipe.setPtr(spAvOut.getPtr());
        return sCtx.Success();
    }

    int createSequencePipe(int nPipe, SPipe pPipes[], SPipe& spPipe) {
        return CPipe::createPipe(PipeMode_Sequence, nPipe, pPipes, spPipe);
    }
    int createParallelPipe(int nPipe, SPipe pPipes[], SPipe& spPipe) {
        return CPipe::createPipe(PipeMode_Parallel, nPipe, pPipes, spPipe);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvFactory, SAvFactory::getClassKey())