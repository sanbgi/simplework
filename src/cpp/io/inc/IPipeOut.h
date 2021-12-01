#ifndef __SimpleWork_IO_IPipeOut_h__
#define __SimpleWork_IO_IPipeOut_h__

#include <typeinfo>

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
        return pull(CFluidAccepterV<Q>(&v), typeid(Q).hash_code());
    }
    template<typename Q> IPipeOut& operator>>(Q& v) {
        pull(CFluidAccepterV<Q>(&v), typeid(Q).hash_code());
        return *this;
    }
    //
    // 从输出管道中推送数值到输入管道中
    //
    template<typename Q> int push(IPipeIn* pAcceptor) {
        int dt = typeid(Q).hash_code();
        return pull(CFluidAccepterR<IPipeIn>(pAcceptor, dt), dt);
    }
    int push(IPipeIn* pAcceptor) {
        return pull(CFluidAccepterR<IPipeIn>(pAcceptor));
    }

protected:
    struct IFluidAccepter {
        template<typename Q> int pushV(Q v) {
            return push(CFluidData(typeid(Q).hash_code(), &v));
        }
        template<typename Q> int pushR(Q& v) {
            return push(CFluidData(typeid(Q).hash_code(), &v));
        }
    private:
        virtual int push(IPipeIn::IFluidData* pFluidData) = 0;
    };

private:
    template<typename Q> struct CFluidAccepterV : public IFluidAccepter {
        int m_dt;
        Q* m_ptr;
        int push(IPipeIn::IFluidData* pFluidData) {
            if(pFluidData->getDt() == m_dt) {
                (*m_ptr) = (Q*)pFluidData->getPtr();
                return Error::Success;
            }
            return Error::Failure;
        }
        CFluidAccepterV(Q* ptr){
            m_dt = typeid(Q).hash_code();
            m_ptr = ptr;
        }
        operator IFluidAccepter*() { return this; }
    };
    template<typename Q> struct CFluidAccepterR : public IFluidAccepter {
        int m_dt;
        Q* m_ptr;
        int push(IPipeIn::IFluidData* pFluidData) {
            if(m_dt != 0 && m_dt != pFluidData->getDt()) {
                return Error::Failure;
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