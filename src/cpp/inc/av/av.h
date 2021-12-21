#ifndef __SimpleWork_AV_h__
#define __SimpleWork_AV_h__

#define SIMPLEWORK_WITHOUTAPI

#include "../core/core.h"
#include "../math/math.h"

#define SIMPLEWORK_AV_NAMESPACE sw::av
#define SIMPLEWORK_AV_NAMESPACE_ENTER namespace sw { namespace av {
#define SIMPLEWORK_AV_NAMESPACE_LEAVE }}

#include "PAvFrame.h"
#include "PAvSample.h"
#include "PAvStreaming.h"

#include "SAvFilter.h"
#include "SAvFactory.h"

#endif//__SimpleWork_AV_h__