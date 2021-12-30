#ifndef __SimpleWork_AV_PAvFrame_H__
#define __SimpleWork_AV_PAvFrame_H__

#include "av.h"
#include "PAvSample.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
//
// 注意音视频数据的结构说明，其结构特征如下
//
// 音视频帧信息
//
//      1，每一个视频或音频输入输出分解为多个流(PAvStreaming)
//      2，每一个流由多个帧组成(PAvFrame)；
//      3，每一帧包含一个（视频图片）或多个样本点(音频采样点)(PAvSample)
//
//  帧数据格式：
//
//      1，一帧数据由多个数据平面组成，平面数量为nPlanes，指针为ppPlanes[i]
//      2，每一个平面的数据大小，可以通过计算获得，具体为pPlaneLinesize[i] * nHeight;
//
//  举例说明：
//
//      1，视频的RGB格式数据，则由一个长度为height*width*3的unsigned char数组组成
//      2，音频的单声道U8数据，则由一个长度为：样本数*1*1的unsigned char数组组成
//  但是，由于多种原因，很多时候，数据无法由单一数组组成，比如：YUV视频，则有三个分量
//  组成，而且数据长度不一致，所以，才会有多个数据平面的概念，每一个平面存储一个分量，
//  实际在视频文件中，采用YUV格式的占绝大多数，因为其压缩效率比RGB等要高。
//
//  注意事项：
//
//      这个数据结构，只能在一个调用栈中，才有效，过了一个调用栈，则指针都可能变为非法
//  指针，或者内容会被修改。所以，数据的访问，务必在一个调用栈中完成。（这么设计的原因是，
//  在进行数据流的处理中，数据量很大，频繁的分配和释放内存，效率低。为了提高效率，音视频
//  流，一个流大多复用同一个缓冲区，而不会去为每一帧数据分配内存）
//
//
struct PAvFrame {
    SIMPLEWORK_PDATAKEY(PAvFrame, "sw.av.AvFrame")


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
    //
    // 当前帧样本数量
    //      视频：图像宽度，pPlaneLineSize[0]/nWidth，就是每一个数据平面，单像素字节数
    //      音频：单声道采样点数，pPlaneLineSize[0]/nWidth就是每一个数据面上，音频采样点字节数
    //
    int nWidth;
    //
    //  数据的高度，pPlaneLineSize[0] * nHeight，就是ppPlane[0] 的 字节数
    //      视频，图像高度
    //      音频：始终为1
    //
    int nHeight;
    //
    // 样本数据包含多少数据面
    //
    int nPlanes;
    //
    // 指针数组，其长度为和nPlanes一致，其每一项内容为指向数据面的指针
    //
    unsigned char** ppPlanes;
    //
    // 数据长度数组，其长度为和nPlanes一致，单位为字节数，取值为：
    //
    //      视频：一行视频数据所占用字节数
    //      音频：一行音频数据占用的字节数，由于音频始终只有一行，所以这个也就是一个数据平面的字节数
    //
    int* pPlaneLineSizes;

    //
    // 典型的帧数据访问器类型定义
    //
    typedef IVisitor<const PAvFrame*>* FVisitor;
};

SIMPLEWORK_AV_NAMESPACE_LEAVE


#endif//__SimpleWork_AV_PAvFrame_H__