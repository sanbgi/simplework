#ifndef __SimpleWork_IO_IPipeIn_h__
#define __SimpleWork_IO_IPipeIn_h__

SIMPLEWORK_IO_NAMESPACE_ENTER

//
//
// 输入管道（可往管道推送数据）
//
//
SIMPLEWORK_INTERFACE_ENTER(IPipeIn, sw::core::IObject, "sw.io.IPipeIn", 011130)

    template<typename Q> int pushV(Q v) {
        return push(CFluidData(BasicType::getType<Q>(), &v));
    }
    template<typename Q> int pushR(Q& v) {
        return push(CFluidData(BasicType::getType<Q>(), &v));
    }
    template<typename Q> IPipeIn& operator<<(Q v) {
        push(CFluidData(BasicType::getType<Q>(), &v));
        return *this;
    }

protected:
    struct IFluidData {
    public:
        virtual EBasicType getDt() = 0;
        template<typename Q> bool isType() {
            return getDt() == BasicType::getType<Q>();
        }
        template<typename Q> Q& getValue() {
            if( BasicType::getType<Q>() == getDt() ) {
                return *((Q*)getPtr());
            }
            return *((Q*)nullptr);
        }
    private:
        virtual void* getPtr() = 0;
        friend class IPipeIn;
        friend class IPipeOut;
    };
    int putRaw(IFluidData* pFluidData) {
        return push(CFluidData(pFluidData->getDt(), pFluidData->getPtr()));
    }

private:
    class CFluidData : public IFluidData {
    public:
        EBasicType getDt() { return m_dt; }
        void* getPtr() { return m_ptr; }
        operator IFluidData*() { return this; }
    public:
        CFluidData(EBasicType dt, void* ptr){
            m_dt = dt;
            m_ptr = ptr;
        }
    protected:
        EBasicType m_dt;
        void* m_ptr;
    };

private:
    virtual int push(IFluidData* pFluidData) = 0;
    friend struct IPipeOut;

SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_IPipeIn_h__