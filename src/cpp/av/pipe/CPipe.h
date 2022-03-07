#include "../Av.h"
#include <vector>

using namespace sw;

enum EPipeMode {
    PipeMode_Sequence,
    PipeMode_Parallel
};

class CPipe : public CObject, IPipe {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int pushData(const PArg& rData, IVisitor<const PArg&,int>* pReceiver);

public:
    static int createPipe(EPipeMode eMode, int nPipe, SPipe pPipes[], SPipe& spPipe);

private:
    std::vector<SPipe> m_arrPipes;
    EPipeMode m_eMode;
};
