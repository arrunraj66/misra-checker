#if FEATURE_X
static int a;
#elif defined(FEATURE_Z) && FEATURE_W
static int b;
#endif
