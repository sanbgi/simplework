#ifndef __SimpleWork_IO_SArchivable_h__
#define __SimpleWork_IO_SArchivable_h__

#include "io.h"
#include "SIoFactory.h"

SIMPLEWORK_IO_NAMESPACE_ENTER

class SArchive;

SIMPLEWORK_INTERFACECLASS_ENTER0(Archivable)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.av.IArchivable", 220112)

        //
        // 获取版本号
        //
        virtual int getClassVer() = 0;

        //
        // 获取名字
        //
        virtual const char* getClassName() = 0;

        //
        // 获取对象的类名，用于序列化时构建对象
        //
        virtual const char* getClassKey() = 0;

        //
        // 序列化
        //
        virtual int toArchive(const SArchive& ar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static int saveBinaryFile(const char* szFileName, const SArchivable& spAr) {
        return SIoFactory::getFactory()->saveArchive(szFileName, spAr);
    }

    static int loadBinaryFile(const char* szFileName, SArchivable& spAr) {
        return SIoFactory::getFactory()->loadArchive(szFileName, spAr);
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(Archivable)

SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_SArchivable_h__