// Standard library.
#include <vector>
#include <limits>

// Third party.
#include "TColgp_HArray1OfPnt.hxx"
#include "TColStd_HArray1OfBoolean.hxx"
#include "TColgp_HArray1OfVec.hxx"
#include "GeomAPI_Interpolate.hxx"
#include "Geom_Circle.hxx"
#include "Geom_TrimmedCurve.hxx"
#include "GeomConvert.hxx"
#include "Standard_Handle.hxx"
#include "GC_MakeCircle.hxx"
#include "GC_MakeArcOfCircle.hxx"
#include "gp_Ax1.hxx"
#include "gp_Vec.hxx"
#include "gp_Dir.hxx"
#include "gp_Pnt.hxx"
#include "gp_Circ.hxx"

// Library public.
#include "geometric_primitives.hxx"
#include "toolpath.hxx" 

Curve::~Curve() = default;

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
InterpolatedCurve::InterpolatedCurve(const std::vector<Point3D>& interpolation_points,
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

    this->representation = interpolation.Curve(); 
}

/*
    Defines an arc that is a part of a circle.
    
    Arguments:
        arc_endpoints: The endpoints of the arc. These points must both lie on
                           the ball defined by the center and the radius. 
        center:        The centerpoint of the circle.
        radius:        The radius of the circle.
*/
ArcOfCircle::ArcOfCircle(const std::pair<Point3D, Point3D>& arc_endpoints,
                         const Point3D& center,
                         const double radius)
{
    // In order to build the circle with the chosen API, it's necessary to
    //     define the circle's axis. The axis lies at the center of the circle
    //     and the z-direction of the axis defines the normal to the circle.
    // The user passes the center point of the circle, but it's necessary to
    //     compute the z-direction using the cross product.
    // Warning: The normal to the resulting circle is not explicitly chosen
    //     - it ends up being whatever the result of the cross product is.
    const gp_Pnt arc_endpoint1 {arc_endpoints.first[0], arc_endpoints.first[1], arc_endpoints.first[2]};
    const gp_Pnt arc_endpoint2 {arc_endpoints.second[0], arc_endpoints.second[1], arc_endpoints.second[2]};
    const gp_Pnt centerpoint {center[0], center[1], center[2]};

    gp_Vec center_to_ep1 {centerpoint, arc_endpoint1};
    const gp_Vec center_to_ep2 {centerpoint, arc_endpoint2};
    center_to_ep1.Cross(center_to_ep2);

    const gp_Ax1 circle_axis {centerpoint, center_to_ep1};

    const GC_MakeCircle circle_maker {circle_axis, radius};
    const Handle(Geom_Circle) circle {circle_maker.Value()};

    // Now use the circle to build the arc. 
    const GC_MakeArcOfCircle arc_maker {(*circle).Circ(), arc_endpoint1, arc_endpoint2, true}; 
    const Handle(Geom_TrimmedCurve) arc {arc_maker.Value()};

    this->representation = GeomConvert::CurveToBSplineCurve(arc);
}
