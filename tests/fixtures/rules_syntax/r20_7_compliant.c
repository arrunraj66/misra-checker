extern int helper(int a);
#define SQUARE(x) ((x) * (x))
#define CALL(x) helper(x)
#define STR(x) #x
#define CAT(a, b) a##b
