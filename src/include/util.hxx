// ***** LIBRARY PRIVATE *****

#pragma once

// Standard library.
#include <cmath>

#define FP_EQUALS_TOLERANCE pow(10, -7)

bool compare_fp(double fp1, double fp2, double eps=FP_EQUALS_TOLERANCE);
