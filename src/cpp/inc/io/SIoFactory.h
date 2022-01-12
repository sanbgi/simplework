#ifndef __SimpleWork_IO_SIoFactory_h__
#define __SimpleWork_IO_SIoFactory_h__

#include "io.h"

SIMPLEWORK_IO_NAMESPACE_ENTER

class SIoArchivable;

SIMPLEWORK_INTERFACECLASS_ENTER(IoFactory, "sw.io.IoFactory")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.io.IIoFactory", 220112)

        //
        // 保存文件
        //
        virtual int saveArchive(const char* szFileName, const SIoArchivable& spAr) = 0;

        //
        // 读取文件
        //
        virtual int loadArchive(const char* szFileName, SIoArchivable& spAr) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SIoFactory& getFactory() {
        static SIoFactory g_factory = sw::core::SObject::createObject<SIoFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(IoFactory)

SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_SIoFactory_h__