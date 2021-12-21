#ifndef __SimpleWork_AV_AvFactory_h__
#define __SimpleWork_AV_AvFactory_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvFactory, "sw.av.AvFactory")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFactory", 011220)

        virtual int openAvFileReader(const char* szFileName, SPipe& spPipe) = 0;
        virtual int openVideoCapture(const char* szDeviceName, SPipe& spPipe) = 0;
        virtual int openAudioCapture(const char* szDeviceName, SPipe& spPipe) = 0;
        virtual int openAvConverter(PAvSample targetSample, SPipe& spPipe) = 0;
        virtual int openWindow(const char* szWindowName, int nWidth, int nHeight, SPipe& spPipe) = 0;
        virtual int openSpeaker(const char* szDeviceName, SPipe& spPipe) = 0;
        virtual int openAvFileWriter(const char* szFileName, SPipe& spPipe) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SAvFactory& getAvFactory() {
        static SAvFactory g_factory = SObject::createObject<SAvFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvFactory)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvFactory_h__