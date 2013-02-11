/***************************************************************************
 *  Description:
 *      Format and indent XML files
 *
 *  Arguments:
 *      XML filename
 *
 *  Returns:
 *      See sysexits.h
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sysexits.h>
#include "xml-format.h"

int     main(int argc,char *argv[])

{
    switch(argc)
    {
	case    2:
	    break;
	default:
	    fprintf(stderr,"Usage: %s\n",argv[0]);
	    break;
    }
    return xml_format(argv[1]);
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

int xml_format(const char *filename)

{
    FILE    *infile;
    
    if ( (infile = fopen(filename,"r")) == NULL )
	return EX_NOINPUT;
    
    process_block(infile, 0);
    fclose(infile);
    return EX_OK;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

int     process_block(FILE *infile, int indent)

{
    int     ch,
	    col = 0,
	    indent_step = 4;
    char    tag_name[MAX_TAG_LEN+1];
    char    output_buff[MAX_LINE_LEN+1];
    tag_list_t  tags;
    
    tag_list_load(&tags);
    
    while ( (ch = getc(infile)) != EOF )
    {
	if ( ch == '<' )
	{
	    read_tag(infile, tag_name, MAX_TAG_LEN);
	    switch ( tag_type(&tags, tag_name) )
	    {
		/*
		 *  If sectioning tag (chapter, section) change indent
		 *  and start new line.
		 */
		case    SECTIONING_TAG:
		    if ( *tag_name != '/' ) /* Closing tag */
		    {
			// Dump current line and put tag on next line
			flush_line(output_buff, infile, indent, &col);
			buffer_tag(output_buff, &col, tag_name);
			flush_line(output_buff, infile, indent, &col);
			INCREASE(indent, indent_step);
			flush_line(output_buff, infile, indent, &col);
		    }
		    else
		    {
			// Dump current line and put tag on next line
			swallow_space(infile);
			DECREASE(indent,indent_step);
			flush_line(output_buff, infile, indent, &col);
			buffer_tag(output_buff, &col, tag_name);
			flush_line(output_buff, infile, indent, &col);
		    }
		    break;

		/*
		 *  If block tag (para, note, etc.) just start new line.
		 */
		case    BLOCK_TAG:
		    if ( *tag_name != '/' ) /* Closing tag */
		    {
			flush_line(output_buff, infile, indent, &col);
			buffer_tag(output_buff, &col, tag_name);
			flush_line(output_buff, infile, indent, &col);
		    }
		    else
		    {
			flush_line(output_buff, infile, indent, &col);
			buffer_tag(output_buff, &col, tag_name);
			flush_line(output_buff, infile, indent, &col);
		    }
		    break;
		
		case    LINE_TAG:
		    if ( *tag_name != '/' ) /* Closing tag */
		    {
			buffer_tag(output_buff, &col, tag_name);
		    }
		    else
		    {
			buffer_tag(output_buff, &col, tag_name);
			flush_line(output_buff, infile, indent, &col);
		    }
		    break;
		
		case    COMMENT_TAG:
		    buffer_tag(output_buff, &col, tag_name);
		    flush_line(output_buff, infile, indent, &col);
		    break;
		
		/*
		 *  If inline tag, do nothing.
		 */
		default:
		    buffer_tag(output_buff, &col, tag_name);
	    }
	}
	else if ( isspace(ch) )
	{
	    output_buff[col] = ' ';

	    if ( col > MAX_COLS - 2 )
	    {
		int     save_col, c;
		
		// Back up to previous space and terminate line there
		for (c = col - 1; ( c > 0 ) && !isspace(output_buff[c]); --c )
		    ;
		if ( c > 0 )
		{
		    save_col = col;
		    col = c;
		    flush_line(output_buff, infile, indent, &col);
		    memmove(output_buff + col, output_buff + c + 1, save_col - c);
		    col += save_col - c - 1;
		}
		else
		{
		    flush_line(output_buff, infile, indent, &col);
		}
	    }

	    if ( swallow_space(infile) == EOF )
	    {
		flush_line(output_buff, infile, indent, &col);
		return EX_OK;
	    }
	    ++col;
	}
	else
	    // Check for end of buff
	    output_buff[col++] = ch;
    }
    return EX_OK;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

int     read_tag(FILE *infile, char *tag_name_ptr, int max_tag_len)

{
    // FIXME: Check for MAX_TAG_LEN
    while ( (*tag_name_ptr = getc(infile)) != '>' )
	++tag_name_ptr;
    *tag_name_ptr = '\0';
    return EX_OK;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

tag_t   tag_type(tag_list_t *tags, const char *tag_name)

{
    int     c;
    
    if ( tag_name[0] == '!' )
	return COMMENT_TAG;
    
    /*
     *  Tag name may start at position 0 or 1 (after '/')
     */
    for (c = 0; c <= 1; ++c)
    {
	if ( bsearch(tag_name + c, tags->sectioning_tags, tags->sectioning_tag_count,
	    sizeof(char *), (int (*)(const void *,const void *))memptrcmp) != NULL )
	    return SECTIONING_TAG;
	
	if ( bsearch(tag_name + c, tags->block_tags, tags->block_tag_count,
	    sizeof(char *), (int (*)(const void *,const void *))memptrcmp) != NULL )
	    return BLOCK_TAG;
	
	if ( bsearch(tag_name + c, tags->line_tags, tags->line_tag_count,
	    sizeof(char *), (int (*)(const void *,const void *))memptrcmp) != NULL )
	    return LINE_TAG;
    }
    return INLINE_TAG;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

void    flush_line(char *output_buff, FILE *infile, int indent, int *col)

{
    output_buff[*col] = '\0';
    if ( !strblank(output_buff) )
	puts(output_buff);
    //fflush(stdout);
    memset(output_buff, ' ', indent);
    swallow_space(infile);
    *col = indent;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-09  Jason Bacon Begin
 ***************************************************************************/

int     swallow_space(FILE *infile)

{
    int     ch;
    
    while ( isspace(ch = getc(infile)) )
	;
    if ( !feof(infile) )
	ungetc(ch, infile);
    return ch;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-10  Jason Bacon Begin
 ***************************************************************************/

void    buffer_tag(char *output_buff, int *col, const char *tag_name)

{
    //fprintf(stderr, "*col = %d\n", *col);
    snprintf(output_buff + *col, MAX_LINE_LEN - *col, "<%s>", tag_name);
    *col += strlen(tag_name) + 2;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-10  Jason Bacon Begin
 ***************************************************************************/

size_t  read_string_list(const char *filename, char *str_array[],
	size_t max_strings, size_t max_strlen)

{
    size_t  list_size = 0;
    FILE    *infile;
    char    string[max_strlen+1];
    
    if ( (infile = fopen(filename, "r")) == NULL )
	return -1;
    while ( read_string(infile, string, max_strlen) != 0 )
    {
	str_array[list_size++] = strdup(string);
    }
    fclose(infile);
    qsort(str_array, list_size, sizeof(char *),
	(int (*)(const void *,const void *))strptrcmp);
    /*for (c = 0; c < list_size; ++c)
	puts(str_array[c]);*/
    return list_size;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-10  Jason Bacon Begin
 ***************************************************************************/

size_t  read_string(FILE *infile, char *string, size_t max_strlen)

{
    char    *p;
    
    for (p = string; ( (*p = getc(infile)) != '\n' ); ++p)
	;
    *p = '\0';
    return p - string;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-10  Jason Bacon Begin
 ***************************************************************************/

int     tag_list_load(tag_list_t *tags)

{
    tags->sectioning_tag_count = read_string_list("sectioning-tags.txt",
	tags->sectioning_tags, MAX_TAGS, MAX_TAG_LEN);
    tags->block_tag_count = read_string_list("block-tags.txt",
	tags->block_tags, MAX_TAGS, MAX_TAG_LEN);
    tags->line_tag_count = read_string_list("line-tags.txt",
	tags->line_tags, MAX_TAGS, MAX_TAG_LEN);
    return EX_OK;
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-10  Jason Bacon Begin
 ***************************************************************************/

int     strptrcmp(char **s1, char **s2)

{
    return strcmp(*s1, *s2);
}


/***************************************************************************
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  History: 
 *  Date        Name        Modification
 *  2013-02-10  Jason Bacon Begin
 ***************************************************************************/

int     memptrcmp(char *s1, char **s2)

{
    //printf("%s %d\n", s1, strlen(s1));
    return memcmp(s1, *s2, strlen(*s2));
}

