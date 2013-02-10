
#define MAX_TAG_LEN     4096
#define MAX_LINE_LEN    4096
#define MAX_COLS        80

typedef enum
{
    SECTIONING_TAG,
    BLOCK_TAG,
    INLINE_TAG,
    LINE_TAG,
    COMMENT_TAG
}   tag_t;

#define MAX(x,y)    ((x) > (y) ? (x) : (y))
#define MIN(x,y)    ((x) < (y) ? (x) : (y))

#define INCREASE(indent,step)    (indent = MIN((indent+step),40))
#define DECREASE(indent,step)    (indent = MAX((indent-step),0))

#include "protos.h"

