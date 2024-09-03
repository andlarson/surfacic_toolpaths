/*
    Functionality to construct curves in space through which a machine tool moves. 
*/

// Third party.
#include "GProp_PEquation.hxx"
#include "TColgp_HArray1OfPnt.hxx"
#include "TColStd_HArray1OfBoolean.hxx"

// Library private.
#include "src/include/tool_curve.hxx"
#include "src/include/util.hxx"

/* 
   ============================================================================
                                    ToolCurve 
   ============================================================================ 
*/ 

/*
    Defines a curve in space via interpolation that lies on a plane parallel to
        the global xy axis. Does not allow full control over the interpolation 
        process.
    
    Arguments:
        points:   Points to be interpolated. The points must all lie on a single
                      plane that has constant z value.
        tangents: A collection of (idx, tangent vector) pairs. Each (idx,
                      tangent vector) pair specifies the tangent at the point
                      at index idx in the list of points to be interpolated. 
                  When a curve is interpolated between the points, these tangents
                      will be honored. 
                  A tangent need not be specified for every point. However, a 
                      tangent must be specified for the first point that composes
                      the curve.
                  All tangent vectors must have zero z component. The tool curve
                      must lie on a single plane.
*/
ToolCurve::ToolCurve(const std::vector<Point3D>& points,
                     const std::vector<std::pair<uint64_t, Vec3D>>& tangents)
{
    // ------------------------------------------------------------------------ 
    //                      Imperfect Precondition Checking 
    
    assert(points.size() > 1);
    assert(tangents.size() >= 1);
    assert(tangents.size() <= points.size());
    
    bool first_point_tangent {false};
    for (const auto& tangent : tangents)
    {
        assert(tangent.first < points.size());
        assert(compare_fp(tangent.second[2], 0));
        if (tangent.first == 0)
            first_point_tangent = true;
    }
    assert(first_point_tangent);

    // ------------------------------------------------------------------------ 
    
    // ------------------------------------------------------------------------ 
    //                          Convert to OCCT format
    
    Handle(TColgp_HArray1OfPnt) points_to_interpolate {new TColgp_HArray1OfPnt(1, points.size())};
    for (uint64_t i {0}; i < points.size(); ++i)
    {
        (*points_to_interpolate)[i + 1] = gp_Pnt(points[i][0], 
                                                 points[i][1], 
                                                 points[i][2]);
    }

    Handle(TColStd_HArray1OfBoolean) tangent_bools {new TColStd_HArray1OfBoolean(1, points.size())};
    tangent_bools->Init(false);
    
    Handle(TColgp_HArray1OfVec) tangent_vecs {new TColgp_HArray1OfVec(1, points.size())};
    for (uint64_t i {0}; i < tangents.size(); ++i)
    {
        (*tangent_vecs)[tangents[i].first + 1] = gp_Vec(tangents[i].second[0],
                                                        tangents[i].second[1],
                                                        tangents[i].second[2]);
        (*tangent_bools)[i + 1] = true;
    }

    // ------------------------------------------------------------------------ 
   
    
    // ------------------------------------------------------------------------ 
    //                    Imperfect Precondition Checking 
    
    // Done after conversion to OCCT because some of this uses OCCT functionality.
    const GProp_PEquation property_tester(*points_to_interpolate, FP_EQUALS_TOLERANCE);
    assert(property_tester.IsPlanar() || property_tester.IsLinear());
    assert(compare_fp(points[0][2], points[1][2]));

    // ------------------------------------------------------------------------ 
    
    this->points_to_interpolate = points_to_interpolate;
    this->tangent_bools = tangent_bools;
    this->tangents = tangent_vecs;
}
