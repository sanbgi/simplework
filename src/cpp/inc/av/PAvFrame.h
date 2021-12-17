#ifndef __SimpleWork_AV_PAvFrame_H__
#define __SimpleWork_AV_PAvFrame_H__

#include "av.h"
#include "PAvSample.h"
#include "PAvStreaming.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 
struct PAvStreaming;

//
//
// 视频帧信息
//
//      1，每一个视频或音频输入输出分解为多个流(PAvStreaming)
//      2，每一个流由多个帧组成(PAvFrame)；
//      3，每一帧包含一个（视频图片）或多个样本点(音频采样点)(PAvSample)
//
//  数据格式：
//      视频数据主要以帧的形式存储，每一帧数据由多个存储平面组成，其平面数据的指针
//  存储在planeDatas中，对于音视频，如果没有被分解为不同的分量，则一般只有第一个指
//  针有效存储了所有的数据，比如：
//      1，视频的RGB格式数据，则由一个长度为height*width*3的unsigned char数组组成
//      2，音频的单声道U8数据，则由一个长度为：样本数*1*1的unsigned char数组组成
//  但是，由于多种原因，很多时候，数据无法由单一数组组成，比如：YUV视频，则有三个分量
//  组成，而且数据长度不一致，所以，才会有多个数据平面的概念，每一个平面存储一个分量，
//  实际在视频文件中，采用YUV格式的占绝大多数，因为其压缩效率比RGB等要高。
//
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

    //
    // 时间戳，对应视频流的时钟频率，比如：时钟频率是1000，时间戳是10，则表示10/1000秒
    //
    int timeRate;
    long timeStamp;

    //
    // 当前帧样本数量
    //      音频：单声道采样点数
    //      视频：1
    //
    int samples;

    //
    // 样本数据包含多少数据面
    //
    int samplePlanes;
    //
    // 指针数组，其长度为和samplePlanes一致，其每一项内容为指向数据面的指针
    //
    unsigned char** planeDatas;
    //
    // 数据长度数组，其长度为和samplePlanes一致，单位为字节数，取值为：
    //
    //      视频：一行视频所占用字节数
    //      音频：一个声道数据占用的字节数
    //
    int* planeLineSizes;

    typedef IVisitor<const PAvFrame*>* FVisitor;
};

SIMPLEWORK_AV_NAMESPACE_LEAVE


#endif//__SimpleWork_AV_PAvFrame_H__