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
    static SNnNetwork createRotNetwork();
    static SNnNetwork createShiftNetwork();
    static SNnNetwork createGlobalPollNetwork();
    static SNnNetwork createLeNet_5(bool isStandardNet=false);
    static void runLearn();
    static void runTest();
    static void runFile();
};

#endif//__SimpleWork_NeuralWork_H__