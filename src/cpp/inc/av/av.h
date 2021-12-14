#ifndef __SimpleWork_AV_h__
#define __SimpleWork_AV_h__

#define SIMPLEWORK_WITHOUTAPI

#include "../core/core.h"
#include "../math/math.h"

#define SIMPLEWORK_AV_NAMESPACE sw::av
#define SIMPLEWORK_AV_NAMESPACE_ENTER namespace sw { namespace av {
#define SIMPLEWORK_AV_NAMESPACE_LEAVE }}

#include "EAvSampleType.h"
#include "CAvSampleMeta.h"
#include "EAvStreamingType.h"
#include "SAvFrame.h"
#include "SAudioDevice.h"
#include "SVideoDevice.h"
#include "SAvStreaming.h"
#include "SAvIn.h"
#include "SAvOut.h"

//#include "IPipeIn.h"
//#include "IPipeOut.h"

#endif//__SimpleWork_AV_h__