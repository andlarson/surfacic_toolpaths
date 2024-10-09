#pragma once

// Third party.
#include "Geom_BSplineCurve.hxx"
#include "Standard_Handle.hxx"

// Library public.
#include "tool_curve.hxx"

class ToolCurve::ToolCurve_Impl
{
public:
    Handle(Geom_BSplineCurve) curve; 
};
