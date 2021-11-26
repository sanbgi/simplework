#ifndef __SimpleWork_ICoreModule_h__
#define __SimpleWork_ICoreModule_h__

namespace SimpleWork {

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACE_ENTER(SimpleWork, ICoreModule, IModule)

    //
    // 创建模块
    //
    virtual SmartPtr<IObject> createModule(const char* szModuleKey) = 0;

SIMPLEWORK_INTERFACE_LEAVE

}//namespace SimpleWork

#endif//__SimpleWork_CoreApi_h__