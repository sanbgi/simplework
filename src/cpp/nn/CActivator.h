#ifndef __SimpleWork_NN_CActivation_H__
#define __SimpleWork_NN_CActivation_H__

#include <vector>
#include "nn.h"
using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CActivator{
public:
    virtual double activate(double v) = 0;
    virtual double deactivate(double dOutput, double dDelta) = 0;

public:
    static CActivator* getLeRU();
};

#endif//__SimpleWork_NN_CActivation_H__
