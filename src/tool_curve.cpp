// Third party.
#include "GProp_PEquation.hxx"
#include "TColgp_HArray1OfPnt.hxx"
#include "TColStd_HArray1OfBoolean.hxx"
#include "TColgp_HArray1OfVec.hxx"
#include "GeomAPI_Interpolate.hxx"
#include "Standard_Handle.hxx"

// Library public.
#include "geometric_primitives.hxx"
#include "tool_curve.hxx"

// Library private.
#include "tool_curve_p.hxx"
#include "util_p.hxx"

/*
    *****************************************************************************
                            ToolCurve: PImpl Forwarding
    *****************************************************************************
*/

ToolCurve::ToolCurve()
    : pimpl{std::make_unique<ToolCurve_Impl>()}
{
}

ToolCurve::~ToolCurve() = default;

/*
    *****************************************************************************
                                 InterpolatedToolCurve  
    *****************************************************************************
*/

/*
    Defines a curve in space via interpolation.
    
    Arguments:
        points:   Points to be interpolated. 
        tangents: A collection of (idx, tangent vector) pairs. Each (idx,
                      tangent vector) pair specifies the tangent at the point
                      at index idx in the list of points to be interpolated. 
                  When a curve is interpolated between the points, these tangents
                      will be honored. 
                  A tangent need not be specified for every point. However, a 
                      tangent must be specified for the first point that composes
                      the curve.
*/
InterpolatedToolCurve::InterpolatedToolCurve(const std::vector<Point3D>& interpolation_points,
                                             const std::vector<std::pair<uint64_t, Vec3D>>& tangents)
{
    // ------------------------------------------------------------------------ 
    //                      Imperfect Precondition Checking 
    
    assert(interpolation_points.size() > 1);
    assert(tangents.size() >= 1);
    assert(tangents.size() <= interpolation_points.size());
    
    bool first_point_tangent {false};
    for (const auto& tangent : tangents)
    {
        assert(tangent.first < interpolation_points.size());
        if (tangent.first == 0)
            first_point_tangent = true;
    }
    assert(first_point_tangent);

    // ------------------------------------------------------------------------ 
    //                          Convert to OCCT types 
    
    Handle(TColgp_HArray1OfPnt) points_to_interpolate {new TColgp_HArray1OfPnt(1, interpolation_points.size())};
    for (uint64_t i {0}; i < interpolation_points.size(); ++i)
    {
        (*points_to_interpolate)[i + 1] = gp_Pnt(interpolation_points[i][0], 
                                                 interpolation_points[i][1], 
                                                 interpolation_points[i][2]);
    }

    Handle(TColStd_HArray1OfBoolean) tangent_bools {new TColStd_HArray1OfBoolean(1, interpolation_points.size())};
    tangent_bools->Init(false);
    
    Handle(TColgp_HArray1OfVec) tangent_vecs {new TColgp_HArray1OfVec(1, interpolation_points.size())};
    for (uint64_t i {0}; i < tangents.size(); ++i)
    {
        (*tangent_vecs)[tangents[i].first + 1] = gp_Vec(tangents[i].second[0],
                                                        tangents[i].second[1],
                                                        tangents[i].second[2]);
        (*tangent_bools)[i + 1] = true;
    }

    // ------------------------------------------------------------------------ 
    //                              Interpolating 

    GeomAPI_Interpolate interpolation(points_to_interpolate, false,
                                      std::numeric_limits<double>::min());
    
    interpolation.Load(*tangent_vecs, tangent_bools);

    interpolation.Perform();
    interpolation.IsDone();
    Handle(Geom_BSplineCurve) interpolated_curve {interpolation.Curve()};

    this->pimpl->curve = interpolated_curve; 
}

/*
    *****************************************************************************
                                     RadialToolCurve 
    *****************************************************************************
*/

RadialToolCurve::RadialToolCurve(const std::pair<Point3D, Point3D>& intersection_points,
                                 const double radius)
{
}
