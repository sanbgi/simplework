#include "COptimizer.h"
#include <map>
#include <string>
#include <math.h>

using namespace std;
using namespace sw;

SCtx sCtx("COptimizer");

#include "optimizers/CAdamOptimizer.h"
#include "optimizers/CAdagradOptimizer.h"
#include "optimizers/CRMSPropOptimizer.h"
#include "optimizers/CAdaModOptimizer.h"
#include "optimizers/CNAdamOptimizer.h"
#include "optimizers/CAdaMaxOptimizer.h"

typedef int (*FCreateOptimizer)(SOptimizer& spOptimizer);
map<string, FCreateOptimizer> s_mapDoubleOptimizerFs = {
    { "adam", CAdamOptimizer<double>::createOptimizer },
    { "adamax", CAdaMaxOptimizer<double>::createOptimizer },
    { "nadam", CNAdamOptimizer<double>::createOptimizer },
    { "adamop", CAdaModOptimizer<double>::createOptimizer },
    { "adagrad", CAdagradOptimizer<double>::createOptimizer },
    { "rmsprop", CRMSPropOptimizer<double>::createOptimizer },
};

map<string, FCreateOptimizer> s_mapFloatOptimizerFs = {
    { "adam", CAdamOptimizer<float>::createOptimizer },
    { "adamax", CAdaMaxOptimizer<float>::createOptimizer },
    { "nadam", CNAdamOptimizer<float>::createOptimizer },
    { "adamop", CAdaModOptimizer<float>::createOptimizer },
    { "adagrad", CAdagradOptimizer<float>::createOptimizer },
    { "rmsprop", CRMSPropOptimizer<float>::createOptimizer },
};

int COptimizer::getOptimizer(const char* szOptimizer, unsigned int idType, SOptimizer& spOptimizer) {
    if(szOptimizer != nullptr && *szOptimizer != 0) {
        map<string, FCreateOptimizer>* pMap;
        if(idType == CBasicData<double>::getStaticType()) {
            pMap = &s_mapDoubleOptimizerFs;
        }else
        if(idType == CBasicData<float>::getStaticType()) {
            pMap = &s_mapFloatOptimizerFs;
        }else{
            return sCtx.error("不认识的数据类型");
        }
        map<string, FCreateOptimizer>::iterator it = pMap->find(szOptimizer);
        if( it != pMap->end() ) {
            return (*it->second)(spOptimizer);
        }
        return sCtx.error((std::string("不支持的优化器: ") + szOptimizer).c_str());
    }

    if(idType == CBasicData<double>::getStaticType()) {
        return CNAdamOptimizer<double>::createOptimizer(spOptimizer);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return CNAdamOptimizer<float>::createOptimizer(spOptimizer);
    }
    
    return sCtx.error("不认识的数据类型");
}