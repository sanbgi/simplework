#ifndef __SimpleWork_AV_PAvFrame_H__
#define __SimpleWork_AV_PAvFrame_H__

#include "av.h"
#include "PAvSample.h"
#include "PAvStreaming.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 
struct PAvStreaming;

//
// 视频帧纯数据（音频或视频帧）
//
struct PAvFrame {

    //
    // 样本信息
    //
    PAvSample sampleMeta;

    //
    // 视频流ID，类型
    //
    int streamingId;
    EAvStreamingType streamingType;

    //
    // 时间戳，对应视频流的时钟频率，比如：时钟频率是1000，时间戳是10，则表示10/1000秒
    //
    int timeRate;
    long timeStamp;

    //
    // 样本数据包含多少数据面
    //
    int samplePlanes;
    //
    // 指针数组，其长度为和samplePlanes一致，其每一项内容为指向数据面的指针
    //
    unsigned char** planeDatas;
    //
    // 数据长度数组，其长度为和samplePlanes一致，单位为字节数
    //
    int* planeSizes;
};

SIMPLEWORK_AV_NAMESPACE_LEAVE


#endif//__SimpleWork_AV_PAvFrame_H__