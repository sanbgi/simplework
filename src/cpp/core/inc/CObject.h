#ifndef __SimpleWork_CObject__h__
#define __SimpleWork_CObject__h__

__SimpleWork_Core_Namespace_Enter__

//
//  对象基类：负责实现IObject接口查询能力，所有对象都要从这个基类派生。
//
class CObject : public IObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
        SIMPLEWORK_INTERFACE_ENTRY(IObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CObject__h__