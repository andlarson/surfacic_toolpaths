#pragma once

// Standard library.
#include <memory>
#include <vector>

// Library public.
#include "geometric_primitives.hxx"

struct ToolCurve
{
    // PImpl idiom. 
    class ToolCurve_Impl;
    std::unique_ptr<ToolCurve_Impl> pimpl;

    ToolCurve();
    virtual ~ToolCurve() = 0;
};

class InterpolatedToolCurve : public ToolCurve
{
public:
    InterpolatedToolCurve(const std::vector<Point3D>& interpolation_points,
                          const std::vector<std::pair<uint64_t, Vec3D>>& tangents);
};

class RadialToolCurve : public ToolCurve
{
public:
    RadialToolCurve(const std::pair<Point3D, Point3D>& intersection_points,
                    const double radius);
};
