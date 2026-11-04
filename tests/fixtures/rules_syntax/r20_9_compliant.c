#define FEATURE_Y 1
#if FEATURE_Y
static int a;
#endif
#if defined(FEATURE_UNSET) || defined FEATURE_Y
static int b;
#endif
