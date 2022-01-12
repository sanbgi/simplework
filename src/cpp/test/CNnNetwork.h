#ifndef __SimpleWork_NeuralWork_H__
#define __SimpleWork_NeuralWork_H__

#include "../inc/SimpleWork.h"

using namespace sw;
using namespace sw;

class CNnNetwork {
public:
    static void run();
    static SNnNetwork createNetwork();
    static SNnNetwork createRotNetwork();
    static void runFile();
};

#endif//__SimpleWork_NeuralWork_H__