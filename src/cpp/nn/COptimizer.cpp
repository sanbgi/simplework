#include "COptimizer.h"
#include <map>
#include <string>
#include <math.h>

using namespace std;

SCtx sCtx("COptimizer");

#include "optimizers/CAdamOptimizer.h"
#include "optimizers/CAdagradOptimizer.h"
#include "optimizers/CRMSPropOptimizer.h"
#include "optimizers/CAdaModOptimizer.h"
#include "optimizers/CNAdamOptimizer.h"
#include "optimizers/CAdaMaxOptimizer.h"

typedef int (*FCreateOptimizer)(SOptimizer& spOptimizer);
map<string, FCreateOptimizer> s_mapOptimizerFs = {
    { "adam", CAdamOptimizer::createOptimizer },
    { "adamax", CAdaMaxOptimizer::createOptimizer },
    { "nadam", CNAdamOptimizer::createOptimizer },
    { "adamop", CAdaModOptimizer::createOptimizer },
    { "adagrad", CAdagradOptimizer::createOptimizer },
    { "rmsprop", CRMSPropOptimizer::createOptimizer },
};

int COptimizer::getOptimizer(const char* szOptimizer, SOptimizer& spOptimizer) {
    if(szOptimizer) {
        map<string, FCreateOptimizer>::iterator it = s_mapOptimizerFs.find(szOptimizer);
        if( it != s_mapOptimizerFs.end() ) {
            return (*it->second)(spOptimizer);
        }
        return sCtx.error((std::string("不支持的优化器: ") + szOptimizer).c_str());
    }
    return CNAdamOptimizer::createOptimizer(spOptimizer);
}