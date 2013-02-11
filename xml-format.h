
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

#define MAX_TAGS    4096

typedef struct
{
    size_t  sectioning_tag_count;
    size_t  block_tag_count;
    size_t  line_tag_count;
    char    *sectioning_tags[MAX_TAGS];
    char    *block_tags[MAX_TAGS];
    char    *line_tags[MAX_TAGS];
}   tag_list_t;

#define MAX(x,y)    ((x) > (y) ? (x) : (y))
#define MIN(x,y)    ((x) < (y) ? (x) : (y))

#define INCREASE(indent,step)    (indent = MIN((indent+step),40))
#define DECREASE(indent,step)    (indent = MAX((indent-step),0))

#include "protos.h"

