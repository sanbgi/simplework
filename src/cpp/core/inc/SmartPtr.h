#ifndef __SimpleWork_AutoPrt__h__
#define __SimpleWork_AutoPrt__h__

namespace SimpleWork {

//
// 对象的智能指针定义，仅适用于从IObject派生的接口
//
template<typename TInterface> class SmartPtr : IObject::IPtrForceSaver {

public:
    SmartPtr(){
        _ptr = nullptr;
    }
    SmartPtr(TInterface* ptr) {
        initPtr(ptr);
    }
    SmartPtr(const SmartPtr& src) {
        initPtr(src._ptr);
    }
    template<typename Q> SmartPtr(Q* pPtr) : _ptr(nullptr) {
        initPtr(pPtr);
    }
    template<typename Q> SmartPtr(const SmartPtr<Q>& src) : _ptr(nullptr) {
        initPtr(src.getPtr());
    }
    ~SmartPtr() {
        releasePtr();
    }
    const SmartPtr& operator=(const SmartPtr& src) {
        assignPtr(src._ptr);
        return *this;
    }

public:
    TInterface* getPtr() const { 
        return _ptr; 
    }

public:
    bool isNullptr() const { 
        return _ptr == nullptr; 
    }
    TInterface* operator->() const {
        return _ptr; 
    }
    operator bool(){
        return _ptr != nullptr;
    }
    operator IObject::IPtrForceSaver*() {
        return this;
    }

//
// 
//  私有成员及函数区域
// 
//
private:
    TInterface* _ptr = nullptr;

private://IConvertObjectContainer
    //
    //  这个函数非常不安全，因为会将一个指针强制转化为接口，目前只有IObject::__swConvertTo函数调用这个接口，用
    //  于返回强制转化为目标接口的指针，而转化工作在宏定义SIMPLEWORK_INTERFACE_ENTRY中。
    //
    int forceSetPtr(void* pPtr) {
        return assignPtr((TInterface*)pPtr);
    }

private:
    void initPtr(TInterface* pPtr = nullptr) {
        if( pPtr ) {
            _ptr = pPtr;
            _ptr->__swAddRef();
        }
    }
    template<typename Q> void initPtr(Q* pPtr = nullptr) {
        if( pPtr ) {
            pPtr->__swConvertTo(TInterface::getInterfaceKey(), this);
        }
    }
    void releasePtr() {
        if( _ptr ) {
            _ptr->__swDecRef();
            _ptr = nullptr;
        }
    }
    template<typename Q> int assignPtr(Q* pPtr = nullptr) {
        if( _ptr != pPtr ) {
            releasePtr();
            initPtr(pPtr);
        }
        return Error::Success;
    }
};
typedef SmartPtr<IObject> IObjectPtr;

}//namespace SimpleWork

#endif//__SimpleWork_AutoPrt__h__