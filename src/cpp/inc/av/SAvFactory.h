#ifndef __SimpleWork_AV_AvFactory_h__
#define __SimpleWork_AV_AvFactory_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

class SAvIn;
class SAvOut;
class SAvFrame;
class SAvNetwork;

//
//
//  音视频工厂，目前音视频工厂对于所有音视频，包括输入、输出、过滤等，为了追求新能，都是使用
//  的管道模式，其中：
//
//      输入视频管道 - 文件、摄像头、Mic等；
//      输出视频管道 - 文件、显示屏、音响等；
//      过滤视频管道 - 帧格式转化器；
//
//      典型的输入视频管道，其管道输入参数目前是任意参数，而输出则是一帧音视频。如果是第一帧音
//  视频，则在这一帧输出前，会输出所有的流信息PAvStreaming。
//
//      典型的输出视频管道，其管道输入是音视频帧，当然在第一帧之前，可以输入视频流信息PAvStreaming；
//  而其输出往往没有。
//
//
SIMPLEWORK_INTERFACECLASS_ENTER(AvFactory, "sw.av.AvFactory")

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFactory", 011220)

        //
        //  创建视频文件读取管道，视频文件读取管道
        //      输入数据：无论输入什么数据？
        //      输出数据：
        //          1，如果是第一帧调用，则依次输出所有的PAvStreaming及第一帧音频或视频PAvFrame
        //          2，如果不是第一帧调用，则输出一帧音频或视频
        //
        virtual int openAvFileReader(const char* szFileName, SAvIn& spIn) = 0;
        //
        //  创建摄像头管道，输入输出同视频文件管道
        //
        virtual int openVideoCapture(const char* szDeviceName, SPipe& spPipe) = 0;
        //
        //  创建音频录制管道，输入输出同视频文件管道
        //
        virtual int openAudioCapture(const char* szDeviceName, SPipe& spPipe) = 0;
        //
        //  音视频帧转化器
        //      输入数据：一帧音视频
        //      输出数据：输出转化为指定格式后的帧
        //
        virtual int openAvFrameConverter(PAvSample targetSample, SAvNetwork& spNetwork) = 0;
        //
        //  创建windows窗口管道，其管道
        //      输入管道：PAvFrame
        //      输出管道：无
        //
        virtual int openWindow(const char* szWindowName, int nWidth, int nHeight, SAvOut& spAvOut) = 0;
        //
        //  创建音响管道
        //      输入管道：
        //          PAvStreaming：初始化音频设备
        //          PAvFrame：需要播放的音频帧
        //      输出管道：无
        //
        virtual openSpeaker(const char* szDeviceName, const PAvSample& sampleMeta, SAvOut& spAvOut) = 0;
        //
        //  创建视频文件写入管道
        //      输入管道：
        //          PAvStreaming（初始化音视频流信息）和
        //          PAvFrame 音视频帧，nullptr意味着音视频；
        //      输出管道：
        //          无
        //
        virtual int openAvFileWriter(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings, SAvOut& spAvWriter) = 0;
        //
        // 将多个管道串行成也给新管道
        //
        virtual int createSequencePipe(int nPipe, SPipe pPipes[], SPipe& spPipe) = 0;
        //
        // 创建多个管道并行的管道
        //
        virtual int createParallelPipe(int nPipe, SPipe pPipes[], SPipe& spPipe) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SAvFactory& getAvFactory() {
        static SAvFactory g_factory = SObject::createObject<SAvFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(AvFactory)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvFactory_h__