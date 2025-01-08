#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LENGTH 255

int main() {
  char input[MAX_INPUT_LENGTH];
  // get user input
  if (fgets(input, MAX_INPUT_LENGTH, stdin)) {
    // remove \n
    if (input[strlen(input) - 1] == '\n') {
      input[strlen(input) - 1] = '\0';
    }

    // tokenize, print, and free memory
    TokenList tokens = tokenize(input);
    for (int i = 0; i < tokens.count; i++) {
      printf("%s\n", tokens.tokens[i]);
      free(tokens.tokens[i]);
    }

    // free array
    free(tokens.tokens);
  }

  return 0;
}
