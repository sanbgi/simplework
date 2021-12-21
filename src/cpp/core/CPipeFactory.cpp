#include "Core.h"
#include <vector>

__SimpleWork_Core_Namespace_Enter__

enum EPipeMode {
    PipeMode_Sequence,
    PipeMode_Parallel
};

class CPipe : public CObject, IPipe {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int pushData(const PData& rData, IVisitor<const PData&>* pReceiver) {
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
                            return SError::ERRORTYPE_SUCCESS;
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
                    if( (*it)->pushData(rData, pReceiver) != SError::ERRORTYPE_SUCCESS ) {
                        return SError::ERRORTYPE_FAILURE;
                    }
                }
                return SError::ERRORTYPE_SUCCESS;
            }
            break;

            default:
                return SError::ERRORTYPE_FAILURE;
        }
        return SError::ERRORTYPE_SUCCESS;
    }

public:
    static int createPipe(EPipeMode eMode, int nPipe, SPipe pPipes[], SPipe& spPipe) {
        CPointer<CPipe> spPointer;
        CObject::createObject<CPipe>(spPointer);
        for( int i=0; i<nPipe; i++) {
            spPointer->m_arrPipes.push_back(pPipes[i]);    
        }
        spPointer->m_eMode = eMode;
        spPipe = spPointer.getObject();
        return spPipe ? SError::ERRORTYPE_SUCCESS : SError::ERRORTYPE_FAILURE;
    }

private:
    std::vector<SPipe> m_arrPipes;
    EPipeMode m_eMode;
};

class CPipeFactory : public CObject, public SPipe::IPipeFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SPipe::IPipeFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createSequencePipe(int nPipe, SPipe pPipes[], SPipe& spPipe) {
        return CPipe::createPipe(PipeMode_Sequence, nPipe, pPipes, spPipe);
    }
    int createParallelPipe(int nPipe, SPipe pPipes[], SPipe& spPipe) {
        return CPipe::createPipe(PipeMode_Parallel, nPipe, pPipes, spPipe);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CPipeFactory, SPipe::SPipeFactory::getClassKey())

__SimpleWork_Core_Namespace_Leave__