#ifndef __SimpleWork_NN_CNnNetwork_H__
#define __SimpleWork_NN_CNnNetwork_H__

#include "nn.h"
#include "COptimizer.h"
#include "CNnOperator.h"
#include <vector>
#include <string>

using namespace sw;
using namespace std;

class CNnNetwork{
public:
    static int saveNetwork(const char* szFileName, const SNnNetwork& spNet);
    static int loadNetwork(const char* szFileName, SNnNetwork& spNet);
};

#endif//__SimpleWork_NN_CNnNetwork_H__
