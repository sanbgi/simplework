#include "CNnNetwork.h"

static SCtx sCtx("CNnNetwork");
int CNnNetwork::saveNetwork(const char* szFileName, const SNnNetwork& spNet){
    return SIoFactory::getFactory()->saveArchive(szFileName, spNet);
}
int CNnNetwork::loadNetwork(const char* szFileName, SNnNetwork& spNet){
    SIoArchivable spObj; 
    if( SIoFactory::getFactory()->loadArchive(szFileName, spObj) != sCtx.success() ) {
        return sCtx.error("读取网络文件错误");
    }
    spNet = spObj;
    if( !spNet ) {
        return sCtx.error("读取的文件并非神经网络文件");
    }
    return sCtx.success();
}