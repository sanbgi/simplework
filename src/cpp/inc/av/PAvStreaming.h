#ifndef __SimpleWork_AV_PAvStreaming_H__
#define __SimpleWork_AV_PAvStreaming_H__

#include "av.h"
#include "PAvSample.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 视频流
//
struct PAvStreaming {

    //
    // 所属的流对应的ID和类型
    //
    int streamingId;
    enum EAvStreamingType {
        AvStreamingType_None,
        AvStreamingType_Video,
        AvStreamingType_Audio,
    } streamingType;

    //
    // 时钟频率
    //
    int timeRate;

    //
    // 样本描述类型
    //
    PAvSample frameMeta;
};
typedef PAvStreaming::EAvStreamingType EAvStreamingType;

SIMPLEWORK_AV_NAMESPACE_LEAVE


#endif//__SimpleWork_AV_PAvStreaming_H__