/*
 * Descriptions:
 * -------------
 * Minimalist & Fast UTF8 Decoder Header
 *
 */
 
#ifndef MINUTF8_H
#define MINUTF8_H

int             utf8_len(const char *s);
void            utf8_dec_ex(int * d, int dl, const char * s);
int *           utf8_dec(const char *s);
int             utf8c(const char * src, const char ** ss,int * move);

#endif
