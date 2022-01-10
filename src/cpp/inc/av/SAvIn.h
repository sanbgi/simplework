#ifndef __SimpleWork_AV_AvIn_h__
#define __SimpleWork_AV_AvIn_h__

#include "av.h"
#include "PAvStreaming.h"
#include "SAvFrame.h"
#include "SAvFactory.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

SIMPLEWORK_INTERFACECLASS_ENTER0(AvIn)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvIn", 220109)

        //
        // 获取流数量
        //
        virtual int getStreamingSize() = 0;

        //
        // 获取流数据
        //
        virtual const PAvStreaming* getStreamingAt(int iPos) = 0;

        //
        // 读取视频帧，如果读取失败，建议调用isCompleted，判断是否时读取完成
        //
        virtual int readFrame(SAvFrame& spAvFrame) = 0;

        //
        // 是否读取完成
        //
        virtual bool isCompleted() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SAvIn openAvFileReader(const char* szFileName) {
        SAvIn spAvIn;
        SAvFactory::getAvFactory()->openAvFileReader(szFileName, spAvIn);
        return spAvIn;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(AvIn)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvIn_h__