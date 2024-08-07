#include "include/util.hxx"

// Dumb floating point comparison.
// Warning: Be very careful. The correct way to compare floating point numbers
//     is highly context dependent.
bool compare_fp(double fp1, double fp2, double eps)
{
    if (abs(fp1 - fp2) < eps)
        return true;
    return false;
}
