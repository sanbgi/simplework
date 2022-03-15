#ifndef __SimpleWork_nn_CNnExtraTensor_H__
#define __SimpleWork_nn_CNnExtraTensor_H__

#include "nn.h"
#include "SNnExtraTensor.h"
#include <vector>

using namespace sw;
using namespace std;
class CNnExtraTensor : public CObject, public ITensor, public IArchivable, public INnExtraTensor {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
        SIMPLEWORK_INTERFACE_ENTRY(INnExtraTensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

public://ITensor
    int getDimension(SDimension& spDim);
    int getDataBuffer(SDeviceMemory& spMemory);
    PDATATYPE getDataType();
    int getDataSize();

private://IArchivable
    int getClassVer() { return 220209; }
    const char* getClassName() { return "NnExtraTensor"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnExtraTensor
    int getResizeData(PNnExtraTensor& rResizeTensor);

public://Factory
    static const char* __getClassKey() { return "sw.nn.NnExtraTensor"; }
    static int createResizeTensor(const PNnExtraTensor& rTenser, STensor& spTensor);

private:
    STensor m_spTensor;
    std::vector<SObject> m_arrExtras;
};

#endif//__SimpleWork_nn_CNnExtraTensor_H__
