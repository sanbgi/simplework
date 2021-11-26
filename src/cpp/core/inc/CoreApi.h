#ifndef __SimpleWork_CoreApi_h__
#define __SimpleWork_CoreApi_h__

//
// 获取当前模块
//
//      每一个模块都必须实现一个自己的getSimpleWorkModule函数，这是整个模块的核心全局函数。大
//  量的模块功能，都需要调用这个函数（调用的地方在各自头文件定义中）。并且系统加载模块时，也需要
//  能找到这个函数，进而获取模块指针，并初始化模块。
//
//      通过实现这个函数，模块甚至都不用依赖于连接SimpleWork库，因为SimpleWork库会自动加载模块，
//  并且会在初始化模块的时候，将系统模块指针作为参数，传递给当前模块，这样，当前模块就可以调用系统
//  模块功能了。
//
//      当然，如果可执行程序，则还是需要调用SimpleWork::getCoreModule进行初始化的。
//
SimpleWork::IModule* getSimpleWorkModule();

namespace SimpleWork {

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACE_ENTER(SimpleWork, ICoreModule, IModule)

    //
    // 创建模块
    //
    virtual int createModule(const char* szModuleKey, IObjectContainer* pContainer) = 0;
    SmartPtr<IObject> createModule(const char* szModuleKey) {
        SmartPtr<IObject> spModule;
        createModule(szModuleKey, &spModule);
        return spModule;
    }

SIMPLEWORK_INTERFACE_LEAVE

//
// 获取系统模块
//
// @pThisModule
//      当前模块指针
//
// @nCompatibleVer
//      当前库的版本号
//
// @remark
//
//      这框架唯一一个输出函数，其它模块可以通过链接这个函数，获取框架相关能力。
//
__SimpleWork_API__ ICoreModule* getCoreModule(int nCompatibleVer=SIMPLEWORK_COMPATIBLE_VER);

}//namespace SimpleWork

#endif//__SimpleWork_CoreApi_h__