#ifndef __SimpleWork_NeuralWork_H__
#define __SimpleWork_NeuralWork_H__

#include "../inc/SimpleWork.h"

using namespace sw::nn;
using namespace sw::core;

class CNeuralNetwork {
public:
    static void run();
    static void run2();
    static void runConv();
    static void runDense();
    static void runPool();
    static SNeuralNetwork createNetwork();
};

#endif//__SimpleWork_NeuralWork_H__