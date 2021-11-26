#ifndef __SimpleWork_IFactory__h__
#define __SimpleWork_IFactory__h__

namespace SimpleWork {

/**
 * 对象工场对象的接口
 */
SIMPLEWORK_INTERFACE_ENTER(SimpleWork, IFactory, IObject)

    /**
     *  创建对象
     */
    virtual IObjectPtr createObject() = 0;

SIMPLEWORK_INTERFACE_LEAVE

}//namespace SimpleWork

#endif//__SimpleWork_IFactory__h__