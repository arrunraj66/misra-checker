/* An #include inside an inactive group is not an active directive. */
#ifdef NEVER_DEFINED_FOR_THIS_TEST
    #include "missing_header.h"
#endif
int x;
