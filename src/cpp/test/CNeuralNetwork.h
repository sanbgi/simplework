#ifndef __SimpleWork_NeuralWork_H__
#define __SimpleWork_NeuralWork_H__

#include "../inc/SimpleWork.h"

using namespace sw::nn;
using namespace sw::core;

class CNeuralNetwork {
public:
    static void run();
    static SNeuralNetwork createNetwork();
    static SNeuralNetwork createRotNetwork();
};

#endif//__SimpleWork_NeuralWork_H__