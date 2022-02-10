#ifndef __SimpleWork_nn_CNnResizeTensor_H__
#define __SimpleWork_nn_CNnResizeTensor_H__

#include "nn.h"
#include "SNnResizeTensor.h"

using namespace sw;
using namespace std;
class CNnResizeTensor : public CObject, public ITensor, public IArchivable, public INnResizeTensor {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
        SIMPLEWORK_INTERFACE_ENTRY(INnResizeTensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

public://ITensor
    int getVer();
    int updateVer();
    int getDimension(SDimension& spDim);
    unsigned int getDataType();
    int getDataSize();
    void* getDataPtr(unsigned int eElementType, int iPos=0);

private://IArchivable
    int getClassVer() { return 220209; }
    const char* getClassName() { return "NnResizeTensor"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnResizeTensor
    int getResizeData(PNnResizeTensor& rResizeTensor);

public://Factory
    static const char* __getClassKey() { return "sw.nn.NnResizeTensor"; }
    static int createResizeTensor(const PNnResizeTensor& rTenser, STensor& spTensor);

private:
    PNnResizeTensor m_src;
};

#endif//__SimpleWork_nn_CNnResizeTensor_H__
