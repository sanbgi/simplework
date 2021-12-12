#ifndef __SimpleWork_AV_AvOut_h__
#define __SimpleWork_AV_AvOut_h__

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvOut, "sw.av.AvOut")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvOut", 211206)
        virtual int putFrame(const AvFrame& frame) = 0;
    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvOutFactory, "sw.av.AvOutFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvOutFactory", 211206)
            virtual AvOut openWindow(const char* szWindowName, int width, int height) = 0;

            virtual AvOut openSpeaker(const char* szName, int sampleRate, int nChannels) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvOutFactory)

public:
    static inline AvOut openWindow(const char* szWindowName, int width, int height) {
        return getFactory()->openWindow(szWindowName, width, height);
    }

    static inline AvOut openSpeaker(const char* szName, int sampleRate, int nChannels) {
        return getFactory()->openSpeaker(szName, sampleRate, nChannels);
    }

private:
    static AvOutFactory& getFactory() {
        static AvOutFactory g_factory = Object::createObject<AvOutFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvOut)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvOut_h__