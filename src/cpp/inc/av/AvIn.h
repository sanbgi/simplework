#ifndef __SimpleWork_AV_AvIn_h__
#define __SimpleWork_AV_AvIn_h__

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvIn, "sw.av.AvIn")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvIn", 211206)
        
    private:
        //打开视频文件
        virtual AvIn openVideoFile(const char* szFileName) = 0;

        //打开摄像头
        virtual AvIn openCapture(const char* szName) = 0;

    public:
        virtual int getStreamingCount() = 0;

        virtual AvStreaming getStreaming(int iStreamingIndex) = 0;

        virtual int getWidth() = 0;

        virtual int getHeight() = 0;

        virtual int getFrame(AvFrame& frame) = 0;

        friend class AvIn;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    static inline AvIn openVideoFile(const char* szFileName) {
        return getFactory()->openVideoFile(szFileName);
    }
    static inline AvIn openCapture(const char* szName) {
        return getFactory()->openCapture(szName);
    }

private:
    static AvIn& getFactory() {
        static AvIn g_factory = Object::createObject(AvIn::getClassKey());
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvIn)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvIn_h__