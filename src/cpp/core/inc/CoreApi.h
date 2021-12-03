#ifndef __SimpleWork_CoreApi_h__
#define __SimpleWork_CoreApi_h__


__SimpleWork_Core_Namespace_Enter__

SIMPLEWORK_INTERFACECLASS_ENTER(CoreApi)

    SIMPLEWORK_OBJECT_INTERFACE_ENTER(IObject, "sw.core.ICoreApi", 211202)
        //
        // 注册工厂
        //
        virtual int registerFactory(const char* szClassKey, const Factory& pFactory) = 0;

        //
        // 根据类名和接口名，创建对象
        // @szClassKey 类名，类似: "sw.math.Tensor", "sw.media.Video", "sw.io.Camera"
        //
        virtual Object createObject(const char* szClassKey) = 0;

        //
        // 根据类名和接口名，创建工厂
        // @szClassKey 类名，类似:  "sw.math.Tensor", "sw.media.Video", "sw.io.Camera"
        //
        virtual Object createFactory(const char* szClassKey) = 0;

        //
        // 如果不想实现自己的模块，则可以调用系统Api创建模块对象
        //
        virtual Object createModule(const char* szModuleKey = nullptr) = 0;

    SIMPLEWORK_OBJECT_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(CoreApi)

__SimpleWork_Core_Namespace_Leave__

#ifndef SIMPLEWORK_WITHOUTAPI

__SimpleWork_Core_Namespace_Enter__

//
// 获取系统核心Api接口
//
// @nCompatibleVer
//      当前库的版本号
//
// @remark
//
//      这框架唯一一个输出函数，其它模块可以通过链接这个函数，获取框架相关能力。
//
__SimpleWork_API__ CoreApi& getCoreApi(int nCompatibleVer=ICoreApi::getInterfaceVer());

__SimpleWork_Core_Namespace_Leave__

#endif//SIMPLEWORK_WITHOUTAPI

#endif//__SimpleWork_CoreApi_h__