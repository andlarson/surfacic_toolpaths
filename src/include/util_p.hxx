#pragma once

// Standard library.
#include <cmath>
#include <string>

const double FP_EQUALS_TOLERANCE {pow(10, -7)};
const int FP_WRITE_PRECISION {15};
const int VERTICES_PER_RECTANGLE {4};
const int VERTICES_PER_TRIANGLE {3};
const int SPACES_PER_TAB {4};
const std::string FOUR_SPACES (SPACES_PER_TAB, ' ');
const std::string EIGHT_SPACES (2 * SPACES_PER_TAB, ' ');
const std::string TWELVE_SPACES (3 * SPACES_PER_TAB, ' ');

bool compare_fp(double fp1, double fp2, double eps=FP_EQUALS_TOLERANCE);
