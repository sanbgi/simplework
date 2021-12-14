#ifndef __SimpleWork_AV_EAvSampleType_H__
#define __SimpleWork_AV_EAvSampleType_H__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 音视频样本类型
//
enum EAvSampleType {
    AvSampleType_None,
    AvSampleType_Video_RGB,
    AvSampleType_Video_RGBA,
    AvSampleType_Audio_U8,
    AvSampleType_Audio_S16,
};

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_EAvSampleType_H__