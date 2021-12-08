#ifndef __SimpleWork_IO_IPipeOut_h__
#define __SimpleWork_IO_IPipeOut_h__

SIMPLEWORK_IO_NAMESPACE_ENTER

struct IPipeIn;

//
//
// 输出管道（可从管道拉取数据）
//
//
SIMPLEWORK_INTERFACE_ENTER(IPipeOut, sw::core::IObject, "sw.io.IPipeOut", 011130)

    //
    // 从输出管道中获取值
    //
    template<typename Q> int getV(Q& v) {
        return pull(CFluidAccepterV<Q>(&v), BasicType::getType<Q>());
    }
    template<typename Q> IPipeOut& operator>>(Q& v) {
        pull(CFluidAccepterV<Q>(&v), BasicType::getType<Q>());
        return *this;
    }
    //
    // 从输出管道中推送数值到输入管道中
    //
    template<typename Q> int push(IPipeIn* pAcceptor) {
        EBasicType dt = BasicType::getType<Q>();
        return pull(CFluidAccepterR<IPipeIn>(pAcceptor, dt), dt);
    }
    int push(IPipeIn* pAcceptor) {
        return pull(CFluidAccepterR<IPipeIn>(pAcceptor));
    }

protected:
    struct IFluidAccepter {
        template<typename Q> int pushV(Q v) {
            return push(CFluidData(BasicType::getType<Q>(), &v));
        }
        template<typename Q> int pushR(Q& v) {
            return push(CFluidData(BasicType::getType<Q>(), &v));
        }
    private:
        virtual int push(IPipeIn::IFluidData* pFluidData) = 0;
    };

private:
    template<typename Q> struct CFluidAccepterV : public IFluidAccepter {
        EBasicType m_dt;
        Q* m_ptr;
        int push(IPipeIn::IFluidData* pFluidData) {
            if(pFluidData->getDt() == m_dt) {
                (*m_ptr) = (Q*)pFluidData->getPtr();
                return Error::SUCCESS;
            }
            return Error::FAILURE;
        }
        CFluidAccepterV(Q* ptr){
            m_dt = BasicType::getType<Q>();
            m_ptr = ptr;
        }
        operator IFluidAccepter*() { return this; }
    };
    template<typename Q> struct CFluidAccepterR : public IFluidAccepter {
        int m_dt;
        Q* m_ptr;
        int push(IPipeIn::IFluidData* pFluidData) {
            if(m_dt != 0 && m_dt != pFluidData->getDt()) {
                return Error::FAILURE;
            }
            return m_ptr->push(pFluidData);
        }
        CFluidAccepterR(Q* ptr, int dt=0){
            m_dt = dt;
            m_ptr = ptr;
        }
        operator IFluidAccepter*() { return this; }
    };
    virtual int pull(IFluidAccepter* pFluidAccepter, int dt = 0) = 0;

SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_IPipeOut_h__