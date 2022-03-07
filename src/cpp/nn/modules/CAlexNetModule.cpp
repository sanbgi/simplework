
#include "module.h"
#include <string>

using namespace sw;
using namespace std;

//
//  参考：https://www.bbsmax.com/A/o75NvYnK5W/
//
static SCtx sCtx("CAlexNetModule");
class CAlexNetModule : public CObject, public INnModule, public IArchivable{

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
    const char* getClassName() { return "AlexNetModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.AlexNetModule"; }

public:
    CAlexNetModule() {
    }
};

int CAlexNetModule::__initialize(const PArg* pData) {
    return sCtx.success();
}

int CAlexNetModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
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

    x = x.conv({11,11,96,1,4,4,nullptr,"relu"});
    x = x.maxpool({3,3,2,2});
    x = x.conv({5,5,256,1,1,1,"same","relu"});
    x = x.maxpool({3,3,2,2});
    x = x.conv({3,3,384,1,1,1,"same","relu"});
    x = x.conv({3,3,384,1,1,1,"same","relu"});
    x = x.conv({3,3,256,1,1,1,"same","relu"});
    x = x.maxpool({3,3,2,2});
    x = x.dense({4096, "relu"});
    x = x.dense({4096, "relu"});
    x = x.dense({1000, "softmax"});

    spOutVar = x;
    return sCtx.success();
}

int CAlexNetModule::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CAlexNetModule, CAlexNetModule::__getClassKey())