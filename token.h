#ifndef _TOKENIZER_H
#define _TOKENIZER_H

// struct for list of tokens
typedef struct {
  char **tokens;
  int count;
} TokenList;

TokenList tokenize(const char *input);

#endif
