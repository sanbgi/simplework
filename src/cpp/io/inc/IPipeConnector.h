#ifndef __SimpleWork_IO_IPipeConnector_h__
#define __SimpleWork_IO_IPipeConnector_h__

SIMPLEWORK_IO_NAMESPACE_ENTER

//
//
// 管道连接器，可以连接到一个输入管道上
//
//
SIMPLEWORK_INTERFACE_ENTER(IPipeConnnector, IObject, "sw.io.IPipeConnnector", 011130)

    virtual int connect(IPipeIn* pIn) = 0;

SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_IPipeConnector_h__