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
    
    while ( (ch = getc(infile)) != EOF )
    {
	if ( ch == '<' )
	{
	    read_tag(infile, tag_name, MAX_TAG_LEN);
	    switch ( tag_type(tag_name) )
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
		    }
		    else
		    {
			// Dump current line and put tag on next line
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
			flush_line(output_buff, infile, indent, &col);
			buffer_tag(output_buff, &col, tag_name);
		    }
		    else
		    {
			buffer_tag(output_buff, &col, tag_name);
			flush_line(output_buff, infile, indent, &col);
		    }
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

tag_t   tag_type(const char *tag_name)

{
    int     c;
    
    if ( tag_name[1] == '!' )
	return COMMENT_TAG;
    
    /* Temporary: replace with tag lists */
    /* Check for both opening and closing tags */
    for (c = 0; c <= 1; ++c)
    {
	if ( ( memcmp(tag_name + c, "chapter", 7) == 0 ) ||
	     ( memcmp(tag_name + c, "section", 7) == 0 ) ||
	     ( memcmp(tag_name + c, "itemizedlist", 12) == 0 ) ||
	     ( memcmp(tag_name + c, "orderedlist", 11) == 0 ) ||
	     ( memcmp(tag_name + c, "note", 4) == 0 ) )
	    return SECTIONING_TAG;
	if ( ( memcmp(tag_name + c, "para", 4) == 0 ) ||
	     ( memcmp(tag_name + c, "listitem", 8) == 0 ) )
	    return BLOCK_TAG;
	if ( ( memcmp(tag_name + c, "title", 5) == 0 ) ||
	     ( memcmp(tag_name + c, "indexterm", 9) == 0 ) )
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

