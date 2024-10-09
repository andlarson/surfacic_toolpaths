#pragma once

// Third party.
#include "TopoDS_Shape.hxx"

// Library public.
#include "tool_curve.hxx"
#include "tool_profile.hxx"

class ToolPath
{
    TopoDS_Shape tool_path;

public:
    ToolPath(const ToolCurve& curve,
             const CylindricalTool& profile,
             const bool display_result);

    void mesh_surface(const double angle, const double deflection);

    void shape_to_stl(const std::string solid_name, 
                      const std::string filepath);
};
