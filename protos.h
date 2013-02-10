/* xml-format.c */
int main(int argc, char *argv[]);
int xml_format(const char *filename);
int process_block(FILE *infile, int indent);
int read_tag(FILE *infile, char *tag_name_ptr, int max_tag_len);
tag_t tag_type(const char *tag_name);
void flush_line(char *output_line, FILE *infile, int indent, int *col);
int swallow_space(FILE *infile);
void buffer_tag(char *output_line, int *col, const char *tag_name);
