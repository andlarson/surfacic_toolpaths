// Standard library.
#include <vector>
#include <limits>
#include <cassert>

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
#include "gp_Vec.hxx"
#include "gp_Pnt.hxx"

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
    Defines an arc that is part of a circle.

    Note:
        OCCT does not document to what precision the three points must actually
            form the arc of a circle. For example, if the end points are specified
            as (1, 0, 0) and (0, 1, 0) and the interior point is specified as
            (.5, .866, 0) is that sufficient? Or would the interior point need
            to be specified as (.5, .8660254038, 0)? I'm not sure what the answer
            to this question is, so it's safest to be as precise as possible.
    
    Assumes:
        (1) The endpoints and the interior point actually define an arc of a
                circle. For example, three collinear points don't define an
                arc of a circle.
        (2) The arc does not form a full circle!
    
    Arguments:
        arc_endpoints:      The two endpoints of the arc. 
        arc_interior_point: The interior point of the arc. 
*/
ArcOfCircle::ArcOfCircle(const std::pair<Point3D, Point3D>& arc_endpoints,
                         const Point3D& arc_interior_point)
{
    const gp_Pnt arc_endpoint1 {arc_endpoints.first.at(0), arc_endpoints.first.at(1), arc_endpoints.first.at(2)};
    const gp_Pnt arc_endpoint2 {arc_endpoints.second.at(0), arc_endpoints.second.at(1), arc_endpoints.second.at(2)};
    const gp_Pnt arc_interior_point1 {arc_interior_point.at(0), arc_interior_point.at(1), arc_interior_point.at(2)};
    // Documentation of point ordering for GC_MakeArcOfCircle is incorrect!
    const GC_MakeArcOfCircle arc_maker {arc_endpoint1, arc_interior_point1, arc_endpoint2}; 
    const Handle(Geom_TrimmedCurve) arc {arc_maker.Value()};

    this->representation = GeomConvert::CurveToBSplineCurve(arc);
}

/*
    Defines a circle.

    Note:
        OCCT does not document to what precision the three points must actually
            form a circle. It's safest to be as precise as possible.

    Assumes:
        (1) The points all lie on a single circle.

    Arguments:
        p1: First point.
        p2: Second point.
        p3: Third point.
*/
Circle::Circle(const Point3D& p1, const Point3D& p2, const Point3D& p3)
{
    const gp_Pnt circle_p1 {p1.at(0), p1.at(1), p1.at(2)};
    const gp_Pnt circle_p2 {p2.at(0), p2.at(1), p2.at(2)};
    const gp_Pnt circle_p3 {p3.at(0), p3.at(1), p3.at(2)};
    const GC_MakeCircle circle_maker {circle_p1, circle_p2, circle_p3};
    const Handle(Geom_Circle) circle {circle_maker.Value()};

    this->representation = GeomConvert::CurveToBSplineCurve(circle);
}
