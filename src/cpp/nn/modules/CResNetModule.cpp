
#include "module.h"
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CResNetModule");
class CResNetModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

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

int CResNetModule::__initialize(const PData* pData) {
    return sCtx.success();
}

SNnVariable CResNetModule::resBlock2(SNnVariable x, int n, int nUpDimension){
    SDimension spInDimension = x.dimension();
    int nXLayers = spInDimension.data()[spInDimension.size()-1];
    int nLayers = nXLayers*nUpDimension;
    while(n-->0) {
        SNnVariable resX = x;
        if(nXLayers != nLayers) {
            resX = resX.conv({3,3,nLayers,1,2,2,"same"});
            x = x.conv({1,1,nLayers,1,2,2,"same","relu"});
            nXLayers = nLayers;
        }else{
            resX = resX.batchNormalize({1.0e-8});
            resX = resX.relu();
            resX = resX.conv({3,3,nLayers,1,1,1,"same"});
        }
        resX = resX.batchNormalize({1.0e-8});
        resX = resX.relu();
        resX = resX.conv({3,3,nLayers,1,1,1,"same"});
        x = x + resX;
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
            resX = resX.conv({1,1,nLayers/4,1,2,2,"same"});
            x = x.conv({1,1,nLayers,1,2,2,"same","relu"});
            nXLayers = nLayers;
        }else{
            resX = resX.batchNormalize({1.0e-8});
            resX = resX.relu();
            resX = resX.conv({1,1,nLayers/4,1,1,1,"same"});
        }
        resX = resX.batchNormalize({1.0e-8});
        resX = resX.relu();
        resX = resX.conv({1,1,nLayers,1,1,1, "same"});
        x = x + resX;
    }
    return x;
}


int CResNetModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("ResNet单元输入参数必须为一个");
    }
    SNnVariable x = spInVars[0];
    x = x.conv({7,7,64,1,2,2,"same","relu"});
    x = x.pool({3,3,2,2,"same"});
    int nResNet = 50;
    switch(nResNet) {
    case 18:
        x = resBlock2(x,2,1);
        x = resBlock2(x,2);
        x = resBlock2(x,2);
        x = resBlock2(x,2);
        break;  
    case 34:
        x = resBlock2(x,3,1);
        x = resBlock2(x,4);
        x = resBlock2(x,6);
        x = resBlock2(x,3);
        break;  
    case 50:
        x = resBlock3(x,3,4);
        x = resBlock3(x,4);
        x = resBlock3(x,6);
        x = resBlock3(x,3);
        break;
    case 101:
        x = resBlock3(x,3,4);
        x = resBlock3(x,4);
        x = resBlock3(x,23);
        x = resBlock3(x,3);
        break;  
    case 152:
        x = resBlock3(x,3,4);
        x = resBlock3(x,8);
        x = resBlock3(x,36);
        x = resBlock3(x,3);
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