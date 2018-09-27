/*
 * assume and assert
 * Example:
 * assume(pre condition);
 * code
 * assert(post condition);
 *
 * If NDEBUG is defined, it will expand into:
 * if(!pre condition) { __builtin_unreachable(); }
 * code
 *
 */


#ifdef assert
#undef assert
#endif

#ifdef assume
#undef assume
#endif

#if !defined(NDEBUG)
// enable asserts
#ifdef SDL
#include <SDL_assert.h>
#define assert(cond) SDL_enabled_assert(cond)
#else
#include <cassert> // fallback
#endif

#define assume(cond) assert(cond)

#elif defined(__GNUC__)
// unreachable
//#define assert(cond) do { if(!(cond)) { __builtin_unreachable(); } } while(0)
#define assume(cond) ((cond) ? ((void)(0)) : __builtin_unreachable())
#define assert(cond) ((void)(0))

#else // todo: add clang support
// just disable
#define assert(cond) ((void)(0))
#define assume(cond) ((void)(0))

//SDL_disabled_assert(cond)
#endif

