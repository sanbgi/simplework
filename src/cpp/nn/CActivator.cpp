#include "CActivator.h"

CActivator* CActivator::getReLU() {
    static class CReLU : public CActivator {
        double activate(double dOutput) {
            return dOutput>=0?dOutput:0;
        }
        double deactivate(double dOutput, double dDelta) {
            //
            //  目标函数 = 求和(delta * delta) / 2，所以，输出相对于目标函数的偏导数刚
            //  好等于 -delta，其中delta = target - current 
            //
            return dOutput>=0?-dDelta:0;
        }
    }s_activator;
    return &s_activator;
}

