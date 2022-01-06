#include "CPipe.h"
#include <vector>

static SCtx sCtx("CPipe");

int CPipe::pushData(const PData& rData, IVisitor<const PData&>* pReceiver) {
    switch (m_eMode)
    {
        case PipeMode_Sequence:
        {
            class CInteralReceiver : public IVisitor<const PData&> {
            public:
                int visit(const PData& rData) {
                    if(iPipe == pArr->size()) {
                        if( pFinalReceiver ) {
                            return pFinalReceiver->visit(rData);
                        }
                        return sCtx.success();
                    }

                    CInteralReceiver receiver;
                    receiver.pFinalReceiver = pFinalReceiver;
                    receiver.iPipe = iPipe+1;
                    receiver.pArr = pArr;
                    return pArr->at(iPipe)->pushData(rData, &receiver);
                }
                int iPipe;
                std::vector<SPipe>* pArr;
                IVisitor<const PData&>* pFinalReceiver;
            }receiver;
            receiver.pArr = &m_arrPipes;
            receiver.iPipe = 0;
            receiver.pFinalReceiver = pReceiver;
            return receiver.visit(rData);
        }
        break;
    
        case PipeMode_Parallel:
        {
            std::vector<SPipe>::iterator it = m_arrPipes.begin();
            for( ; it != m_arrPipes.end(); it++) {
                if( (*it)->pushData(rData, pReceiver) != sCtx.success() ) {
                    return sCtx.error();
                }
            }
            return sCtx.success();
        }
        break;

        default:
            return sCtx.error();
    }
    return sCtx.success();
}

int CPipe::createPipe(EPipeMode eMode, int nPipe, SPipe pPipes[], SPipe& spPipe) {
    CPointer<CPipe> spPointer;
    CObject::createObject<CPipe>(spPointer);
    for( int i=0; i<nPipe; i++) {
        spPointer->m_arrPipes.push_back(pPipes[i]);    
    }
    spPointer->m_eMode = eMode;
    spPipe = spPointer.getObject();
    return spPipe ? sCtx.success() : sCtx.error();
}

