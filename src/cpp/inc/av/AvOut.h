#ifndef __SimpleWork_AV_AvOut_h__
#define __SimpleWork_AV_AvOut_h__

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvOut)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.io.IAvOut", 211206)
        
        virtual int init(const char* szFileName);

        virtual int putFrame(const AvFrame& frame) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvOut)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvOut_h__