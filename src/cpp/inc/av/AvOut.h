#ifndef __SimpleWork_AV_AvOut_h__
#define __SimpleWork_AV_AvOut_h__

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvOut, "sw.av.AvOut")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvOut", 211206)
    private:
        virtual AvOut openWindow(const char* szWindowName, int width, int height) = 0;

    public:
        virtual int putFrame(const AvFrame& frame) = 0;

    friend class AvOut;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    static inline AvOut openWindow(const char* szWindowName, int width, int height) {
        return getFactory()->openWindow(szWindowName, width, height);
    }

private:
    static AvOut& getFactory() {
        static AvOut g_factory = Object::createObject(AvOut::getClassKey());
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvOut)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvOut_h__