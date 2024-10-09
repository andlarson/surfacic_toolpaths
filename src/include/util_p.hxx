#pragma once

// Standard library.
#include <cmath>

#define FP_EQUALS_TOLERANCE pow(10, -7)
#define RECTANGLE_PARAMETRIC_BND_CNT 4
#define VERTICES_PER_RECTANGLE 4

bool compare_fp(double fp1, double fp2, double eps=FP_EQUALS_TOLERANCE);
