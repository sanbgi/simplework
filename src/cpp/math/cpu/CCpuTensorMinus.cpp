#include "operator.h"

using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CCpuTensorMinus");
class CCpuTensorMinus : public CObject, IOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int process(int nArgs, PMemory pArgs[]) {

        

        return sCtx.success();
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCpuTensorMinus, "sw.math.CpuTensorMinus")

SIMPLEWORK_MATH_NAMESPACE_LEAVE