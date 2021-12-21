#ifndef __SimpleWork_AV_PAvStreaming_H__
#define __SimpleWork_AV_PAvStreaming_H__

#include "av.h"
#include "PAvSample.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 视频流
//
struct PAvStreaming {
    SIMPLEWORK_PDATAKEY("sw.av.AvStreaming")

    //
    // 所属的流对应的ID和类型
    //
    int streamingId;

    //
    // 时钟频率
    //
    int timeRate;

    //
    // 流的长度，对于编码的时候，0表示不确定视频时长
    //
    int timeDuration;

    //
    // 样本描述类型
    //
    PAvSample frameMeta;

    //
    // 流数据访问接口
    //
    typedef IVisitor<const PAvStreaming*>* FVisitor;
};

SIMPLEWORK_AV_NAMESPACE_LEAVE


#endif//__SimpleWork_AV_PAvStreaming_H__