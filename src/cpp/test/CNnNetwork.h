#ifndef __SimpleWork_NeuralWork_H__
#define __SimpleWork_NeuralWork_H__

#include "../inc/SimpleWork.h"

using namespace sw;
using namespace sw;

class CNnNetwork {
public:
    static void run();
    static SNnNetwork createNetwork();
    static SNnNetwork createTestNetwork();
    static SNnNetwork createResNetwork();
    static SNnNetwork createGlobalPollNetwork();
    static SNnNetwork createRnnNetwork();
    static SNnNetwork createLayerNetwork();
    static SNnNetwork createLeNet();

    static void runLearn();
    static void runTest();
    static void runTestNetwork();
};

#endif//__SimpleWork_NeuralWork_H__