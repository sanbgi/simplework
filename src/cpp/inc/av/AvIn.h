#ifndef __SimpleWork_AV_AvIn_h__
#define __SimpleWork_AV_AvIn_h__

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvIn)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.io.IAvIn", 211206)
        
        virtual int init(const char* szFileName) = 0;

        virtual int getWidth() = 0;

        virtual int getHeight() = 0;

        virtual int getFrame(AvFrame& frame) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvIn)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvIn_h__