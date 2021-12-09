#ifndef __SimpleWork_Vector_h__
#define __SimpleWork_Vector_h__

using namespace SIMPLEWORK_CORE_NAMESPACE;
SIMPLEWORK_MATH_NAMESPACE_ENTER

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Vector)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IVector", 211202)

    private:
        //
        // 初始化一维张量
        //
        virtual int initVector(Data::DataType eElementType, int nElementSize, void* pElementData = nullptr) = 0;

    public:
        //
        // 获取元素类型
        //
        virtual Data::DataType getDataType() const = 0;

        //
        // 获取元素数量
        //
        virtual int getDataSize() const = 0;

    private:
        //
        // 获取元素数据指针
        //
        virtual const void* getDataPtr(Data::DataType eElementType, int iPos=0) const = 0;

    public:
        //
        // 获取元素数据指针
        //
        template<typename Q> inline const Q* getDataPtr(int iPos=0) const {
            return (Q*)getDataPtr(Data::getType<Q>(), iPos);
        }
        template<typename Q> int getDataValue(int iPos, Q& v) const {
            Q* pV = getDataPtr<Q>(iPos);
            if(pV) {
                v = *pV;
                return Error::ERRORTYPE_SUCCESS;
            }
            return Error::ERRORTYPE_FAILURE;
        }

        friend class Vector;
        friend class CVector;
        friend class CTensor;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    template<typename Q=void> static Vector createVector(int nElementSize, Q* pElementData=nullptr) {
        Vector vector = createVector();
        if(vector) {
            vector->initVector(Data::getType<Q>(), nElementSize, (void*)pElementData);
        }
        return vector;
    }

public:
    static Vector createVector() {
        static Factory g_factory = Object::createFactory("sw.math.Vector");
        return g_factory->createObject();
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Vector)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Vector_h__