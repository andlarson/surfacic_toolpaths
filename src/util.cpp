/*
    Miscellaneous utilities that are generally useful.
*/

// Library private.
#include "src/include/util.hxx"

/* 
   ============================================================================
                           General Utility Functions 
   ============================================================================ 
*/ 

/*
    Dumb floating point comparison.
    Warning: Be very careful. The correct way to compare floating point numbers
        is highly context dependent.
*/
bool compare_fp(double fp1, double fp2, double eps)
{
    if (abs(fp1 - fp2) < eps)
        return true;
    return false;
}
