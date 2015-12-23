#ifndef UTILS_H_
#define UTILS_H_

#ifdef DEBUG
#define PRINTF(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define PRINTF(...) do{ } while ( false )
#endif

#endif
