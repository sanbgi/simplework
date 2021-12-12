#ifndef __SimpleWork_IObject_h__
#define __SimpleWork_IObject_h__

__SimpleWork_Core_Namespace_Enter__

struct IObject{
public:
    const static inline char* getInterfaceKey() { return "sw.core.IObject"; }
    static inline int getInterfaceVer() { return 211212; }

private:
    private:
        virtual int __swAddRef() = 0;
        virtual int __swDecRef() = 0;
        virtual int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, __FunPtrForceSaver funSaver) = 0;
        template<typename T> friend class __CPointer;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IObject_h__