
#include "module.h"
#include <string>

using namespace sw;
using namespace std;

//
// 参考：https://zhuanlan.zhihu.com/p/31852747
//
static SCtx sCtx("CResNetModule");
class CResNetModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PArg* pData);

private://INnModule
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);
    
private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "ResNetModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.ResNetModule"; }

public:
    SNnVariable resBlock2(SNnVariable x, int n, int nUpDimension=2);
    SNnVariable resBlock3(SNnVariable x, int n, int nUpDimension=2);
};

int CResNetModule::__initialize(const PArg* pData) {
    return sCtx.success();
}

SNnVariable CResNetModule::resBlock2(SNnVariable x, int n, int nUpDimension){
    SDimension spInDimension = x.dimension();
    int nXLayers = spInDimension.data()[spInDimension.size()-1];
    int nLayers = nXLayers*nUpDimension;
    while(n-->0) {
        SNnVariable resX = x;
        if(nXLayers != nLayers) {
            x = x.conv({1,1,nLayers,1,2,2,"same",nullptr});
            x = x.batchNormalize({1.0e-8});
            nXLayers = nLayers;

            resX = resX.conv({3,3,nLayers,1,2,2,"same"});
            resX = resX.batchNormalize({1.0e-8});
            resX = resX.relu();
        }else{
            resX = resX.conv({3,3,nLayers,1,1,1,"same"});
            resX = resX.batchNormalize({1.0e-8});
            resX = resX.relu();
        }
        resX = resX.batchNormalize({1.0e-8});
        resX = resX.conv({3,3,nLayers,1,1,1,"same"});
        x = x + resX;
        x = x.relu();
    }
    return x;
}

SNnVariable CResNetModule::resBlock3(SNnVariable x, int n, int nUpDimension){
    SDimension spInDimension = x.dimension();
    int nXLayers = spInDimension.data()[spInDimension.size()-1];
    int nLayers = nXLayers*nUpDimension;
    while(n-->0) {
        SNnVariable resX = x;
        if(nXLayers != nLayers) {
            x = x.conv({1,1,nLayers,1,2,2,"same",nullptr});
            x = x.batchNormalize({1.0e-8});
            nXLayers = nLayers;

            resX = resX.conv({1,1,nLayers/4,1,2,2,"same"});
            resX = resX.batchNormalize({1.0e-8});
            resX = resX.relu();

            /**
             * 个人推荐，

                //池化层缩减尺寸
                x = x.pool({2,2,2,2});
                x = x.linear({nLayers,false,nullptr});
                x = x.batchNormalize({1.0e-8});
                nXLayers = nLayers;

                //如果有缩减层数需求，则通过线性变化缩减尺寸
                resX = x;
                if(nXLayers != nLayers/4) {
                    resX = resX.linear({nLayers/4,false,nullptr});
                    resX = resX.batchNormalize({1.0e-8});
                    resX = resX.relu();
                }
             * 
             */
        }else{
            resX = resX.conv({1,1,nLayers/4,1,1,1,"same"});
            resX = resX.batchNormalize({1.0e-8});
            resX = resX.relu();
        }
        resX = resX.conv({3,3,nLayers/4,1,1,1,"same",nullptr});
        resX = resX.batchNormalize({1.0e-8});
        x = x.relu();

        resX = resX.conv({1,1,nLayers,1,1,1, "same"});
        resX = resX.batchNormalize({1.0e-8});
        x = x + resX;
        x = x.relu();
    }
    return x;
}


int CResNetModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("ResNet单元输入参数必须为一个");
    }
    SNnVariable x = spInVars[0];
    x = x.conv({7,7,64,1,2,2,"same","relu"});
    x = x.maxpool({3,3,2,2,"same"});
    int nResNet = 50;
    switch(nResNet) {
    case 18:
        x = resBlock2(x,2,1);
        x = resBlock2(x,2,2);
        x = resBlock2(x,2,2);
        x = resBlock2(x,2,2);
        break;  
    case 34:
        x = resBlock2(x,3,1);
        x = resBlock2(x,4,2);
        x = resBlock2(x,6,2);
        x = resBlock2(x,3,2);
        break;  
    case 50:
        x = resBlock3(x,3,4);
        x = resBlock3(x,4,2);
        x = resBlock3(x,6,2);
        x = resBlock3(x,3,2);
        break;
    case 101:
        x = resBlock3(x,3,4);
        x = resBlock3(x,4,2);
        x = resBlock3(x,23,2);
        x = resBlock3(x,3,2);
        break;  
    case 152:
        x = resBlock3(x,3,4);
        x = resBlock3(x,8,2);
        x = resBlock3(x,36,2);
        x = resBlock3(x,3,2);
        break;  
    }
    x = x.gap();
    x = x.dense({1000, "softmax"});
    spOutVar = x;
    return sCtx.success();
}

int CResNetModule::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CResNetModule, CResNetModule::__getClassKey())