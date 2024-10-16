// Library public.
#include "toolpath.hxx"
#include "geometric_primitives.hxx"

Line::Line(const Point3D& start_point, const Vec3D& line)
    :line(line), start_point(start_point)
{
}
