#ifndef __SimpleWork_CFactory__h__
#define __SimpleWork_CFactory__h__

#include "core.h"
#include "CObject.h"

__SimpleWork_Core_Namespace_Enter__

template<typename TObject> class CFactory : public IFactory {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
        SIMPLEWORK_INTERFACE_ENTRY(IFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

public://IFactory
    int createObject(SObject& rObject, const PData* pData=nullptr) const {
        CPointer<TObject> spObject;
        CObject::createObject<TObject>(spObject, pData);
        rObject = spObject.getObject();
        return SError::ERRORTYPE_SUCCESS;
    }
};

template<typename TObject> class CSingletonFactory : public IFactory {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER0
        SIMPLEWORK_INTERFACE_ENTRY(IFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE0

public://IFactory
    int createObject(SObject& rObject, const PData* pData=nullptr) const {
        static CPointer<TObject> g_spObject;
        static int r = CObject::createObject<TObject>(g_spObject, pData);
        rObject = g_spObject.getObject();
        return SError::ERRORTYPE_SUCCESS;
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CFactory__h__