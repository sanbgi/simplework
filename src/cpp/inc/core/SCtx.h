#ifndef __SimpleWork_SCtx_h__
#define __SimpleWork_SCtx_h__

#include "core.h"
#include "SError.h"
#include "SCore.h"

__SimpleWork_Core_Namespace_Enter__

//
// 运行上下文接口
//
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
        m_szModule = szModule;
    }
    void warn(const char* szWarn) {
        getCtx()->logWarning(szWarn);
    }
    void message(const char* szMessage) {
        getCtx()->logMessage(szMessage);
    }
    int error(int code, const char* szError = nullptr) {
        getCtx()->logError(szError);
        return code;
    }
    int error(const char* szError = nullptr) {
        getCtx()->logError(szError);
        return SError::ERRORTYPE_FAILURE;
    }
    const inline int success() {
        return SError::ERRORTYPE_SUCCESS;
    }

private:
    IFace* getCtx() {
        if( !(*this) ) {
            SCore::getFactory()->createCtx(m_szModule, *this);
        }
        return getPtr();
    }

private:
    const char* m_szModule; 

SIMPLEWORK_INTERFACECLASS_LEAVE(Ctx)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_SCtx_h__