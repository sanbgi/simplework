#ifndef __SimpleWork_SCtx_h__
#define __SimpleWork_SCtx_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

SIMPLEWORK_INTERFACECLASS_ENTER0(Ctx)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ICtx", 211230)
        //
        //  记录错误
        //
        virtual void logError(const char* szError) = 0;
        //
        //  记录警告
        //
        virtual void logWarning(const char* szWarning) = 0;
        //
        //  记录日志
        //
        virtual void logMessage(const char* szMessage) = 0;
    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(CtxFactory, "sw.core.CtxFactory")
        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ICtxFactory", 211230)
            //
            //  创建日志记录器
            //
            virtual int createCtx(const char* szModule, SCtx& spCtx) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(CtxFactory)

    SCtx(const char* szModule) {
        getFactory()->createCtx(szModule, *this);
    }
    int Warn(const char* szWarn) {
        (*this)->logWarning(szWarn);
        return Success();
    }
    int Error(int code = 1, const char* szError = nullptr) {
        return code;
    }
    int Success() {
        return 0;
    }

private:
    static SCtxFactory& getFactory() {
        static SCtxFactory g_factory = SObject::createObject<SCtxFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(Ctx)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_SCtx_h__