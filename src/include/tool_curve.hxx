// ***** LIBRARY PRIVATE *****

#pragma once

// Third party. 
#include "TColgp_HArray1OfPnt.hxx"
#include "TColStd_HArray1OfBoolean.hxx"
#include "TColgp_HArray1OfVec.hxx"

// Library public.
#include "include/brep_builder.hxx"

class ToolCurve
{
public:
    Handle(TColgp_HArray1OfPnt) points_to_interpolate;
    Handle(TColStd_HArray1OfBoolean) tangent_bools;
    Handle(TColgp_HArray1OfVec) tangents;

    ToolCurve(const std::vector<Point3D>& points,
              const std::vector<std::pair<uint64_t, Vec3D>>& tangents);
};
