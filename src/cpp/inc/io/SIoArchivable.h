#ifndef __SimpleWork_IO_SIoArchivable_h__
#define __SimpleWork_IO_SIoArchivable_h__

#include "io.h"

SIMPLEWORK_IO_NAMESPACE_ENTER

class SIoArchive;

SIMPLEWORK_INTERFACECLASS_ENTER0(IoArchivable)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.av.IIoArchivable", 220112)

        //
        // 获取版本号
        //
        virtual int getVer() = 0;

        //
        // 获取名字
        //
        virtual const char* getName() = 0;

        //
        // 获取对象的类名，用于序列化时构建对象
        //
        virtual const char* getClassKey() = 0;

        //
        // 序列化
        //
        virtual int toVisit(const SIoArchive& ar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(IoArchivable)

SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_SIoArchivable_h__