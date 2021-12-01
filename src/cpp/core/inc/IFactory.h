#ifndef __SimpleWork_IFactory__h__
#define __SimpleWork_IFactory__h__

__SimpleWork_Core_Namespace_Enter__

/**
 * 对象工场对象的接口
 */
SIMPLEWORK_INTERFACE_ENTER(IFactory, IObject, "sw.core.IFactory",011130)

    /**
     *  创建对象
     */
    virtual IObjectPtr createObject() = 0;

SIMPLEWORK_INTERFACE_LEAVE

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IFactory__h__