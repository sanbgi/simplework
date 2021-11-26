#ifndef __SimpleWork_IObjectContainer_h__
#define __SimpleWork_IObjectContainer_h__

namespace SimpleWork {

struct IObject;
struct IObjectContainer {
    virtual int setObject(IObject* pObject) = 0;
};

}//namespace SimpleWork

#endif//__SimpleWork_IObjectContainer_h__