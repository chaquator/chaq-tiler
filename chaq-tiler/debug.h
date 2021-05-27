#ifdef NDEBUG
#define debug(s) ((void)0)
#else
#define debug(s) OutputDebugStringA(s "\n")
#endif
