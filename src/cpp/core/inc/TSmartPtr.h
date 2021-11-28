#ifndef __SimpleWork_AutoPrt__h__
#define __SimpleWork_AutoPrt__h__

__SimpleWork_Core_Namespace_Enter__

//
// 指针强制设置接口，由于设置的是强制转化后的地址，所以，接口不安全，也不能被外界使用
//
struct IPtrForceSaver {
    virtual int forceSetPtr(void* pPtr) = 0;
};


//
// 对象的智能指针定义，仅适用于从IObject派生的接口
//
template<typename TInterface> struct SmartPtr : public IPtrForceSaver {

public:
    SmartPtr(){
        _ptr = nullptr;
    }
    SmartPtr(const SmartPtr& src) : _ptr(nullptr)  {
        initPtr(src._ptr);
    }
    SmartPtr& operator=(const SmartPtr& src) {
        assignPtr(src._ptr);
        return *this;
    }
    //
    // 理论上来说，实现了模板指针构造函数（紧邻的第二个）后，不需要再单独实现这个构造函
    //  数，但当语法 IObjectPtr spObj = nullptr; 里面，构造参数为nullptr，没有类型，
    //  造成编译器无法支持这种语法，所以，但单独定义了一下这个，后续很多情况类似
    //
    SmartPtr(TInterface* ptr) : _ptr(nullptr)  {
        initPtr(ptr);
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
    template<typename Q> SmartPtr& operator=(Q* pSrc) {
        assignPtr(pSrc);
        return *this;
    }
    template<typename Q> SmartPtr& operator=(const SmartPtr<Q>& src) {
        assignPtr(src.getPtr());
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
    operator IPtrForceSaver*() {
        return this;
    }
    operator TInterface*() const {
        return _ptr;
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
        if( _ptr != (TInterface*)pPtr ) {
            releasePtr();
            initPtr(pPtr);
        }
        return Error::Success;
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_AutoPrt__h__