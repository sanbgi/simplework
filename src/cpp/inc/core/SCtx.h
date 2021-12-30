#ifndef __SimpleWork_SCtx_h__
#define __SimpleWork_SCtx_h__

#include "core.h"
#include "SError.h"
#include "SCoreFactory.h"

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

    SCtx(const char* szModule) {
        SCoreFactory::getFactory()->createCtx(szModule, *this);
    }
    void Warn(const char* szWarn) {
        (*this)->logWarning(szWarn);
    }
    void Message(const char* szMessage) {
        (*this)->logMessage(szMessage);
    }
    int Error(int code, const char* szError = nullptr) {
        (*this)->logError(szError);
        return code;
    }
    int Error(const char* szError = nullptr) {
        (*this)->logError(szError);
        return SError::ERRORTYPE_FAILURE;
    }
    int Success() {
        return SError::ERRORTYPE_SUCCESS;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Ctx)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_SCtx_h__