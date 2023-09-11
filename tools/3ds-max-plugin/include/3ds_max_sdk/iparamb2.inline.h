//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2014 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "geom/matrix3.h"
#include "AssetManagement/AssetUser.h"
#include <algorithm> // std::min, std::max

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

//==================================================================================================
// class ParamBlockDesc2
//==================================================================================================

inline const ParamDef* ParamBlockDesc2::GetParamDefByIndex(const unsigned int parameterIndex) const
{
    if(DbgVerify((paramdefs != nullptr) && (parameterIndex < count)))
    {
        return &(paramdefs[parameterIndex]);
    }
    else
    {
        return nullptr;
    }
}

//==================================================================================================
// class IParamBlock2
//==================================================================================================

inline const ParamDef* IParamBlock2::GetParamDefByIndex(const unsigned int parameterIndex) const
{
    ParamBlockDesc2* const pbdesc = const_cast<IParamBlock2*>(this)->GetDesc();
    if(DbgVerify(pbdesc != nullptr))
    {
        return pbdesc->GetParamDefByIndex(parameterIndex);
    }
    else
    {
        return nullptr;
    }
}

inline Color IParamBlock2::GetColor(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    Color val(0.0f, 0.0f, 0.0f);
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline AColor IParamBlock2::GetAColor(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    AColor val(0.0f, 0.0f, 0.0f, 0.0f);
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline Point2 IParamBlock2::GetPoint2(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
	Point2 val(0.0f, 0.0f);
	DbgVerify(GetValue(id, t, val, validity, tabIndex));
	return val;
}

inline Point3 IParamBlock2::GetPoint3(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    Point3 val(0.0f, 0.0f, 0.0f);
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline Point4 IParamBlock2::GetPoint4(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    Point4 val(0.0f, 0.0f, 0.0f, 0.0f);
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline int IParamBlock2::GetInt(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    int val = 0;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline float IParamBlock2::GetFloat(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    float val = 0.0f;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;

}

inline TimeValue IParamBlock2::GetTimeValue(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    TimeValue val = 0;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}



inline const MCHAR* IParamBlock2::GetStr(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    const MCHAR* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline Mtl* IParamBlock2::GetMtl(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    Mtl* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}


inline Texmap* IParamBlock2::GetTexmap(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    Texmap* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline PBBitmap* IParamBlock2::GetBitmap(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    PBBitmap* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline INode* IParamBlock2::GetINode(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    INode* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline ReferenceTarget* IParamBlock2::GetReferenceTarget(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    ReferenceTarget* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline IParamBlock2* IParamBlock2::GetParamBlock2(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    IParamBlock2* val = nullptr;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline Matrix3 IParamBlock2::GetMatrix3(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    Matrix3 val;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

inline MaxSDK::AssetManagement::AssetUser IParamBlock2::GetAssetUser(const ParamID id, const TimeValue t, Interval& validity, const int tabIndex)
{
    MaxSDK::AssetManagement::AssetUser val;
    DbgVerify(GetValue(id, t, val, validity, tabIndex));
    return val;
}

template<typename T>
inline bool IParamBlock2::SetValueByName(const MCHAR* const paramName, const T& value, const TimeValue t, const int tabIndex)
{
    // Find the parameter that matches the given name
    ParamBlockDesc2* const pbdesc = GetDesc();
    if(DbgVerify(pbdesc != nullptr))
    {
        const int param_index = pbdesc->NameToIndex(paramName);
        if(DbgVerify(param_index >= 0))
        {
            const ParamDef* const param_def = pbdesc->GetParamDefByIndex(param_index);
            if(DbgVerify(param_def != nullptr))
            {
                return DbgVerify(SetValue(param_def->ID, t, value, tabIndex) != 0);
            }
        }
    }

    return false;
}

template<typename T>
inline bool IParamBlock2::GetValueByName(const MCHAR* const paramName, const TimeValue t, T& value, Interval& validity, const int tabIndex)
{
    // Find the parameter that matches the given name
    ParamBlockDesc2* const pbdesc = GetDesc();
    if(DbgVerify(pbdesc != nullptr))
    {
        const int param_index = pbdesc->NameToIndex(paramName);
        if(DbgVerify(param_index >= 0))
        {
            const ParamDef* const param_def = pbdesc->GetParamDefByIndex(param_index);
            if(DbgVerify(param_def != nullptr))
            {
                return DbgVerify(GetValue(param_def->ID, t, value, validity, tabIndex) != 0);
            }
        }
    }

    return false;
}

template<typename ValueType>
inline BOOL IParamBlock2::GetValue(ParamID id, TimeValue t, ValueType& v, int tabIndex)
{
	Interval valid = FOREVER;
	return GetValue(id, t, v, valid, tabIndex);
}

inline IParamBlock2::ParameterIterator IParamBlock2::begin()
{
    return ParameterIterator(*this, 0);
}

inline IParamBlock2::ParameterIterator IParamBlock2::end()
{
    return ParameterIterator(*this, NumParams());
}

//==================================================================================================
// class IParamBlock2::ParameterIterator
//==================================================================================================

inline IParamBlock2::ParameterIterator::ParameterIterator(IParamBlock2& param_block, const unsigned int parameter_index)
    : m_param_block(&param_block),
    m_num_params(std::max(param_block.NumParams(), 0)),
    m_parameter_index(std::min(parameter_index, m_num_params))
{
}

inline const ParamDef& IParamBlock2::ParameterIterator::operator*() const
{
    DbgAssert(m_parameter_index < m_num_params);
    const ParamDef* param_def = m_param_block->GetParamDefByIndex(m_parameter_index);
    // The parameter should never be null if the parameter index is valid. The caller should never try to dereference
    // this iterator if the parameter index is out of bounds.
    DbgAssert(param_def != nullptr);
    return *param_def;
}

inline IParamBlock2::ParameterIterator& IParamBlock2::ParameterIterator::operator++()
{
    if(DbgVerify(m_parameter_index < m_num_params))
    {
        ++m_parameter_index;
    }
    return *this;
}

inline IParamBlock2::ParameterIterator IParamBlock2::ParameterIterator::operator++(int)
{
    ParameterIterator ret = *this;
    if(DbgVerify(m_parameter_index < m_num_params))
    {
        ++m_parameter_index;
    }
    return ret;
}

inline bool IParamBlock2::ParameterIterator::operator==(const ParameterIterator& rhs) const
{
    return (m_param_block == rhs.m_param_block) && (m_parameter_index == rhs.m_parameter_index);
}

inline bool IParamBlock2::ParameterIterator::operator!=(const ParameterIterator& rhs) const
{
    return !(*this == rhs);
}

//==================================================================================================
// class ClassDesc2
//==================================================================================================

inline MaxSDK::QMaxParamBlockWidget* ClassDesc2::CreateQtWidget(
    ReferenceMaker& /*owner*/,
    IParamBlock2& /*paramBlock*/,
    const MapID /*paramMapID*/,
    MSTR& /*rollupTitle*/,
    int& /*rollupFlags*/,
    int& /*rollupCategory*/)
{
    // Default implementation returns null.
    // This method only needs to be implemented in plugins with a Qt-based UI that add a Qt widget to any dialog except the Render Settings dialog.
    return nullptr;
}


inline MaxSDK::QMaxParamBlockWidget* ClassDesc2::CreateQtWidget(
    ReferenceMaker& owner,
    IParamBlock2& paramBlock,
    const MapID paramMapID,
    MSTR& rollupTitle,
    int& rollupFlags,
    int& rollupCategory,
    Class_ID& /*tabID*/)
{
    // Default implementation calls the method above, ignoring the tabID parameter.
    // This method only needs to be implemented in plugins with a Qt-based UI that add a Qt widget to the Render Settings dialog.
    return CreateQtWidget(owner, paramBlock, paramMapID, rollupTitle, rollupFlags, rollupCategory);
}


#pragma pop_macro("min")
#pragma pop_macro("max")
