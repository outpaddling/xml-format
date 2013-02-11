/* xml-format.c */
int main(int argc, char *argv[]);
int xml_format(const char *filename);
int process_block(FILE *infile, int indent);
int read_tag(FILE *infile, char *tag_name_ptr, int max_tag_len);
tag_t tag_type(tag_list_t *tags, const char *tag_name);
void flush_line(char *output_buff, FILE *infile, int indent, int *col);
int swallow_space(FILE *infile);
void buffer_tag(char *output_buff, int *col, const char *tag_name);
size_t read_string_list(const char *filename, char *str_array[], size_t max_strings, size_t max_strlen);
size_t read_string(FILE *infile, char *string, size_t max_strlen);
int tag_list_load(tag_list_t *tags);
int strptrcmp(char **s1, char **s2);
int memptrcmp(char *s1, char **s2);
