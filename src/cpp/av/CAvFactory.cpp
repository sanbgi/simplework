
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
    int openAvFileReader(const char* szFileName, SAvIn& spIn) {
        return ffmpeg::CAvIn::createAvFileReader(szFileName, spIn);
    }

    int openVideoCapture(const char* szDeviceName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoCapture(szDeviceName) != sCtx.success() ) {
            return sCtx.error();
        }
        //spPipe.setPtr(spAvIn.getPtr());
        return sCtx.success();
    }

    int openAudioCapture(const char* szDeviceName, SPipe& spPipe) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initAudioCapture(szDeviceName) != sCtx.success() ) {
            return sCtx.error();
        }
        //spPipe.setPtr(spAvIn.getPtr());
        return sCtx.success();
    }

    int openAvFrameConverter(PAvSample targetSample, SAvNetwork& spNetwork) {
        return ffmpeg::CAvFrameConverter::createFilter(targetSample, spNetwork);
    }

    int openWindow(const char* szWindowName, int nWidth, int nHeight, SAvOut& spAvOut) {
        return sdl::CAvOut_SDLWindow::createWindow(szWindowName, nWidth, nHeight, spAvOut);
    }

    int openSpeaker(const char* szDeviceName, const PAvSample& sampleMeta, SAvOut& spAvOut) {
        return sdl::CAvOut_SDLSpeaker::createSpeaker(szDeviceName, sampleMeta, spAvOut);
    }

    int openAvFileWriter(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings, SAvOut& spAvWriter) {
        return ffmpeg::CAvOut::createAvFile(szFileName, nStreamings, pStreamings, spAvWriter);
    }

    int createSequencePipe(int nPipe, SPipe pPipes[], SPipe& spPipe) {
        return CPipe::createPipe(PipeMode_Sequence, nPipe, pPipes, spPipe);
    }
    int createParallelPipe(int nPipe, SPipe pPipes[], SPipe& spPipe) {
        return CPipe::createPipe(PipeMode_Parallel, nPipe, pPipes, spPipe);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CAvFactory, SAvFactory::__getClassKey())