
#include "module.h"
#include <string>

using namespace sw;
using namespace std;

//
//  参考：https://www.bbsmax.com/A/o75NvYnK5W/
//
static SCtx sCtx("CLeNetModule");
class CLeNetModule : public CObject, public INnModule, public IArchivable{

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
    const char* getClassName() { return "LeNetModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.LeNetModule"; }

public:
    CLeNetModule() {
    }
};

int CLeNetModule::__initialize(const PData* pData) {
    return sCtx.success();
}

int CLeNetModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("ResNet单元输入参数必须为一个");
    }
    SNnVariable x = spInVars[0];
    SDimension spInDimension = x.dimension();
    const int* pDimSize = spInDimension.data();
    if( spInDimension.size() == 3 ) {
        if(pDimSize[2] != 1) {
            return sCtx.error("LeNet模型输入需要为单层图像");
        }
    }else if(spInDimension.size() != 2) {
        return sCtx.error("LeNet模型输入为单层图片");
    }

    if( pDimSize[0] != pDimSize[1] ) {
        return sCtx.error("LeNet模型输入尺寸长宽需要相等");
    }

    if(pDimSize[0] == 32 ) {
        x = x.conv({5,5,6,1,1,1,nullptr,nullptr});
        x = x.maxpool({2,2,2,2});
        x = x.sigmod();
        x = x.conv({5,5,16,1,1,1,nullptr,nullptr});
        x = x.maxpool({2,2,2,2});
        x = x.sigmod();
        x = x.dense({120, "sigmod"});
        x = x.dense({84, "sigmod"});
        x = x.dense({10, "softmax"});
    }else if(pDimSize[0] == 28) {
        x = x.conv({5,5,6,1,1,1,"same",nullptr});
        x = x.maxpool({2,2,2,2});
        x = x.sigmod();
        x = x.conv({5,5,16,1,1,1,nullptr,nullptr});
        x = x.maxpool({2,2,2,2});
        x = x.sigmod();
        x = x.dense({120, "sigmod"});
        x = x.dense({84, "sigmod"});
        x = x.dense({10, "softmax"});
    }else{
        return sCtx.error("LeNet模型输入图片尺寸需要为28*28或者32*32");
    }

    spOutVar = x;
    return sCtx.success();
}

int CLeNetModule::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CLeNetModule, CLeNetModule::__getClassKey())