
#include "module.h"
#include <string>

using namespace sw;
using namespace std;

//
//  参考：https://www.bbsmax.com/A/o75NvYnK5W/
//
static SCtx sCtx("CVGGModule");
class CVGGModule : public CObject, public INnModule, public IArchivable{

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
    const char* getClassName() { return "VGGModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.VGGModule"; }

public:
    CVGGModule() {
    }
};

int CVGGModule::__initialize(const PData* pData) {
    return sCtx.success();
}

int CVGGModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("ResNet单元输入参数必须为一个");
    }
    SNnVariable x = spInVars[0];
    SDimension spInDimension = x.dimension();
    const int* pDimSize = spInDimension.data();
    if( spInDimension.size() != 3 ||
        pDimSize[0] != pDimSize[1] ||
        (pDimSize[0] != 224 && pDimSize[0] != 227 )) {
        return sCtx.error("AlexNet模型输入需要为224*224*3 或 227*227*3");
    }

    int nVGG = 16;
    if(nVGG == 16) {
        x = x.conv({3,3,64,1,1,1,"same","relu"});
        x = x.conv({3,3,64,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,128,1,1,1,"same","relu"});
        x = x.conv({3,3,128,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.dense({4096, "relu"});
        x = x.dense({4096, "relu"});
        x = x.dense({1000, "softmax"});
    }else if( nVGG==19 ) {
        x = x.conv({3,3,64,1,1,1,"same","relu"});
        x = x.conv({3,3,64,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,128,1,1,1,"same","relu"});
        x = x.conv({3,3,128,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.conv({3,3,256,1,1,1,"same","relu"});
        x = x.conv({3,3,512,1,1,1,"same","relu"});
        x = x.maxpool({2,2,2,2});
        x = x.dense({4096, "relu"});
        x = x.dense({4096, "relu"});
        x = x.dense({1000, "softmax"});
    }

    spOutVar = x;
    return sCtx.success();
}

int CVGGModule::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CVGGModule, CVGGModule::__getClassKey())