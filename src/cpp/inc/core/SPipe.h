#ifndef __SimpleWork_Core_SPipe_h__
#define __SimpleWork_Core_SPipe_h__


#include "core.h"
#include "PData.h"
#include "IVisitor.h"

__SimpleWork_Core_Namespace_Enter__

//
//
//  管道，推入管道数据，经过管道处理后，返回结果数据，适用于立即处理并且立即获取返回结果的管道模式
//
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Pipe)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IPipe", 011220)

        //
        // 推入数据，并且通过pReceiver获得返回结果
        //
        virtual int pushData(const PData& rData, IVisitor<const PData&>* pReceiver) = 0; 

    SIMPLEWORK_INTERFACE_LEAVE
    
    SIMPLEWORK_INTERFACECLASS_ENTER(PipeFactory, "sw.core.PipeFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.core.IPipeFactory", 211220)
        
            //
            // 将多个管道串行成也给新管道
            //
            virtual int createSequencePipe(int nPipe, SPipe pPipes[], SPipe& spPipe) = 0;

            //
            // 创建多个管道并行的管道
            //
            virtual int createParallelPipe(int nPipe, SPipe pPipes[], SPipe& spPipe) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(PipeFactory)

public:
    static SPipeFactory& getFactory() {
        static SPipeFactory g_factory = SObject::createObject<SPipeFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Pipe)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SPipe_h__