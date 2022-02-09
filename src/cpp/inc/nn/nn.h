#ifndef __SimpleWork_NN_h__
#define __SimpleWork_NN_h__

#include "../core/core.h"
#include "../math/math.h"

#define SIMPLEWORK_NN_NAMESPACE sw
#define SIMPLEWORK_NN_NAMESPACE_ENTER namespace sw { 
#define SIMPLEWORK_NN_NAMESPACE_LEAVE }

#include "PNnConv.h"
#include "PNnPool.h"
#include "PNnDense.h"
#include "PNnRnn.h"
#include "PNnLinear.h"
#include "PNnBatchNormalize.h"
#include "PNnWeight.h"
#include "SNnPipe.h"
#include "SNnUnit.h"
#include "SNnNetwork.h"
#include "SNnState.h"
#include "SNnVariable.h"
#include "SNnVariableSolver.h"
#include "SNnFactory.h"


#endif//__SimpleWork_NN_h__
