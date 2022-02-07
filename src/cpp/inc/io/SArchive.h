#ifndef __SimpleWork_IO_SArchive_h__
#define __SimpleWork_IO_SArchive_h__

#include "io.h"
#include "SIoFactory.h"

SIMPLEWORK_IO_NAMESPACE_ENTER

class SArchivable;

struct IArrayVisitee {
    virtual unsigned int getElementType() = 0;
    virtual int size() = 0;
    virtual const void* data() = 0;
    virtual int getElementBytes() = 0;
    virtual void setArray(int nEle, const void* pEle) = 0;
};

struct IObjectArrayVisitee {
    virtual int size() = 0;
    virtual int getEleAt(int iIndex, SArchivable& spEle) = 0;
    virtual int addEle(const SArchivable& spEle) = 0;
};


SIMPLEWORK_INTERFACECLASS_ENTER0(Archive)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.av.IArchive", 220112)

        //
        // 是否是读入
        //
        virtual bool isReading() = 0;

        //
        // 数据
        //
        virtual int arBlock(const char* szName, unsigned int idType, int nByte, void* pByte, int nMinVer=0, int nMaxVer=99999999) = 0;

        //
        // 数据数组
        //    
        //      注意要确保数据缓冲区尺寸，或者提供pUpdater，供序列化时保存数据 
        //
        virtual int arBlockArray(const char* szName, IArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=99999999) = 0;

        //
        // 对象
        //
        virtual int arObject(const char* szName, SArchivable& spVisitee, int nMinVer=0, int nMaxVer=999999999) = 0;

        //
        // 对象数组
        //
        virtual int arObjectArray(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=999999999) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    //
    // 简单数据类型：bool, int, char, unsigned char ....
    //
    template<typename Q> int arBlock(const char* szName, Q& v, int nMinVer=0, int nMaxVer=999999999 ) const {
        return (*this)->arBlock(szName, sw::CBasicData<Q>::getStaticType(), sizeof(v), &v, nMinVer, nMaxVer);
    }

    //
    // 定长数组
    //
    int arBlockArray(const char* szName, IArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=999999999 ) const {
        return (*this)->arBlockArray(szName, pVisitee, nMinVer, nMaxVer);
    }
    
    //
    // 定长数组
    //
    template<typename Q> int arBlockArray(const char* szName, int nEle, Q* pEle, int nMinVer=0, int nMaxVer=999999999 ) const {
        class CArrayVisitee : IArrayVisitee {
        public:
            unsigned int getElementType() { return sw::CBasicData<Q>::getStaticType(); }
            int size() { return m_nEle; }
            const void* data() { return m_pEle; }
            int getElementBytes() { return sizeof(Q); }
            void setArray(int nEle, const void* pEle) {
                if(nEle == m_nEle) {
                    memcpy(m_pEle, pEle, sizeof(Q)*nEle);
                }
            }

        public:
            CArrayVisitee(int nEle, Q* pEle) {
                m_nEle = nEle;
                m_pEle = pEle;
            }
            operator IArrayVisitee*() {
                return this;
            }

        private:
            int m_nEle;
            Q* m_pEle;
        };
        return (*this)->arBlockArray(szName, CArrayVisitee(nEle, pEle), nMinVer, nMaxVer);
    }

    //
    // 变长数组
    //
    template<typename Q> int visitTaker(const char* szName, int nSize, sw::CTaker<Q*>& spTaker, int nMinVer=0, int nMaxVer = 999999999) const {
        
        class CTakerVisitee : public IArrayVisitee {
        public:
            unsigned int getElementType() { return sw::CBasicData<Q>::getStaticType(); }
            int size() { return m_nEle; }
            const void* data() { return (Q*)m_spTaker; }
            int getElementBytes() { return sizeof(Q); }
            void setArray(int nEle, const void* pEle) {
                m_spTaker.take(new Q[nEle*sizeof(Q)], [](Q* ptr){
                    delete[] ptr;
                });
                memcpy((Q*)m_spTaker, pEle, sizeof(Q)*nEle);
            }

        public:
            CTakerVisitee(int nEle, sw::CTaker<Q*>& spTaker) : m_spTaker(spTaker) {
                m_nEle = nEle;
            }
            operator IArrayVisitee*() {
                return this;
            }

        private:
            int m_nEle;
            sw::CTaker<Q*>& m_spTaker;
        };
        return (*this)->arBlockArray(szName, CTakerVisitee(nSize, spTaker), nMinVer, nMaxVer);
    }

    //
    //
    //
    template<typename Q> int visitString(const char* szName, Q& str, int nMinVer=0, int nMaxVer = 999999999) const {
        class CStringVisitee : IArrayVisitee {
        public:
            unsigned int getElementType() { return sw::CBasicData<char*>::getStaticType(); }
            int size() { return m_pStr->length()+1; }
            const void* data() { return m_pStr->c_str(); }
            int getElementBytes() { return sizeof(char); }
            void setArray(int nEle, const void* pEle) {
                *m_pStr = (char*)pEle;
            }

        public:
            CStringVisitee(Q& str) {
                m_pStr = &str;
            }
            operator IArrayVisitee*() {
                return this;
            }

        private:
            Q* m_pStr;
        };
        return (*this)->arBlockArray(szName, CStringVisitee(str), nMinVer, nMaxVer);
    }

    //
    //
    //
    int arObject(const char* szName, SArchivable& spVisitee, int nMinVer=0, int nMaxVer=999999999) const {
        return (*this)->arObject(szName, spVisitee, nMinVer, nMaxVer);
    }

    template<typename Q>
    int arObject(const char* szName, Q& spObj, int nMinVer=0, int nMaxVer=999999999) const {
        SArchivable arObj = spObj;
        int retcode = (*this)->arObject(szName, arObj, nMinVer, nMaxVer);
        spObj = arObj;
        return retcode;
    }

    //
    //
    //
    int arObjectArray(const char* szName, IObjectArrayVisitee* pVisitee, int nMinVer=0, int nMaxVer=999999999 ) const  {
        return (*this)->arObjectArray(szName, pVisitee, nMinVer, nMaxVer);
    }

    //
    //
    //
    template<typename Q> int arObjectArray(const char* szName, Q& arrEles, int nMinVer=0, int nMaxVer=999999999 ) const  {
        
        class CEleArrayVisitee : IObjectArrayVisitee {
        public:
            unsigned int getElementType() { return sw::CBasicData<char*>::getStaticType(); }
            int size() { return m_pArray->size(); }
            int getEleAt(int iIndex, SArchivable& spEle){
                spEle = (*m_pArray)[iIndex];
                return 0;
            }
            int addEle(const SArchivable& spEle) {
                m_pArray->push_back(spEle);
                return 0;
            }

        public:
            CEleArrayVisitee(Q& arr) {
                m_pArray = &arr;
            }
            operator IObjectArrayVisitee*() {
                return this;
            }

        private:
            Q* m_pArray;
        };
        return (*this)->arObjectArray(szName, CEleArrayVisitee(arrEles), nMinVer, nMaxVer);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Archive)


SIMPLEWORK_IO_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_SArchive_h__