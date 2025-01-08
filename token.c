#include "token.h"
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LENGTH 255

TokenList tokenize(const char *input) {
  TokenList result;
  // allocate memory for token list
  result.tokens = malloc(sizeof(char *) * MAX_INPUT_LENGTH);
  result.count = 0;

  // quotes toggle
  int in_quotes = 0;

  char current_token[MAX_INPUT_LENGTH] = {0};
  int token_index = 0;

  for (int i = 0; i < strlen(input); i++) {
    char c = input[i];

    if (c == '"') {
      // toggle quotes flag
      in_quotes = !in_quotes;
      if (!in_quotes && token_index > 0) {
        // null terminate current token
        current_token[token_index] = '\0';
        // save token
        result.tokens[result.count++] = strdup(current_token);
        // reset index
        token_index = 0;
      }
      continue;
    }

    if (in_quotes) {
      // add char to token inside quotes
      current_token[token_index++] = c;
    } else {
      if ((c >= 0x09 && c <= 0x0D) || c == 0x20 || c == '<' || c == '>' ||
          c == ';' || c == '|' || c == '(' || c == ')') {
        if (token_index > 0) {
          // null terminate, save token, reset token index
          current_token[token_index] = '\0';
          result.tokens[result.count++] = strdup(current_token);
          token_index = 0;
        }
        if (!((c >= 0x09 && c <= 0x0D) || c == 0x20)) {
          // handle special tokens
          char special_token[2] = {c, '\0'};
          result.tokens[result.count++] = strdup(special_token);
        }
      } else {
        // add char to token
        current_token[token_index++] = c;
      }
    }
  }

  if (token_index > 0) {
    // null terminate final token
    current_token[token_index] = '\0';
    // save final token
    result.tokens[result.count++] = strdup(current_token);
  }

  return result;
}
