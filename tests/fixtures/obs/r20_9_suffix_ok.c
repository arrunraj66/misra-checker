/* Integer suffixes in #if conditions are not identifiers. */
#define LIMIT 4U
#if ( LIMIT < 1U )
#error too small
#endif
#if LIMIT > 0x2UL
int big;
#endif
int x;
