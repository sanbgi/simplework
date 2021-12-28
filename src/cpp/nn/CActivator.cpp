#include "CActivator.h"

CActivator* CActivator::getLeRU() {
    static class CLeRU : public CActivator {
        double activate(double v) {
            return v>=0?v:0;
        }
        double deactivate(double dOutput, double dDelta) {
            return -dDelta;
        }
    }s_activator;
    return &s_activator;
}

