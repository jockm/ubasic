/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define DEBUG 0

#if DEBUG
#define DEBUG_PRINTF(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

#include "tokenizer.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//static char const *ptr, *nextptr;

#define MAX_NUMLEN 6

struct keyword_token {
  char *keyword;
  int token;
};

static int current_token = TOKENIZER_ERROR;

static const struct keyword_token keywords[] = {
  {"let", TOKENIZER_LET},
  {"print", TOKENIZER_PRINT},
  {"if", TOKENIZER_IF},
  {"then", TOKENIZER_THEN},
  {"else", TOKENIZER_ELSE},
  {"for", TOKENIZER_FOR},
  {"to", TOKENIZER_TO},
  {"next", TOKENIZER_NEXT},
  {"goto", TOKENIZER_GOTO},
  {"gosub", TOKENIZER_GOSUB},
  {"return", TOKENIZER_RETURN},
  {"call", TOKENIZER_CALL},
  {"rem", TOKENIZER_REM},
  {"peek", TOKENIZER_PEEK},
  {"poke", TOKENIZER_POKE},
  {"end", TOKENIZER_END},
  {NULL, TOKENIZER_ERROR}
};


/*---------------------------------------------------------------------------*/
static int singlechar(ubasic_tokenizer_info *info);
static int get_next_token(ubasic_tokenizer_info *info);
void ubasic_tokenizer_error_print(ubasic_tokenizer_info *info);
int ubasic_tokenizer_finished(ubasic_tokenizer_info *info);
char const *ubasic_tokenizer_pos(ubasic_tokenizer_info *info);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
static int
singlechar(ubasic_tokenizer_info *info)
{
  if(*info->ptr == '\n') {
    return TOKENIZER_CR;
  } else if(*info->ptr == ',') {
    return TOKENIZER_COMMA;
  } else if(*info->ptr == ';') {
    return TOKENIZER_SEMICOLON;
  } else if(*info->ptr == '+') {
    return TOKENIZER_PLUS;
  } else if(*info->ptr == '-') {
    return TOKENIZER_MINUS;
  } else if(*info->ptr == '&') {
    return TOKENIZER_AND;
  } else if(*info->ptr == '|') {
    return TOKENIZER_OR;
  } else if(*info->ptr == '*') {
    return TOKENIZER_ASTR;
  } else if(*info->ptr == '/') {
    return TOKENIZER_SLASH;
  } else if(*info->ptr == '%') {
    return TOKENIZER_MOD;
  } else if(*info->ptr == '(') {
    return TOKENIZER_LEFTPAREN;
  } else if(*info->ptr == '#') {
    return TOKENIZER_HASH;
  } else if(*info->ptr == ')') {
    return TOKENIZER_RIGHTPAREN;
  } else if(*info->ptr == '<') {
    return TOKENIZER_LT;
  } else if(*info->ptr == '>') {
    return TOKENIZER_GT;
  } else if(*info->ptr == '=') {
    return TOKENIZER_EQ;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
get_next_token(ubasic_tokenizer_info *info)
{
  struct keyword_token const *kt;
  int i;

  DEBUG_PRINTF("get_next_token(): '%s'\n", info->ptr);

  if(*info->ptr == 0) {
    return TOKENIZER_ENDOFINPUT;
  }

  if(isdigit(*info->ptr)) {
    for(i = 0; i < MAX_NUMLEN; ++i) {
      if(!isdigit(info->ptr[i])) {
        if(i > 0) {
        	info->nextptr = info->ptr + i;
          return TOKENIZER_NUMBER;
        } else {
          DEBUG_PRINTF("get_next_token: error due to too short number\n");
          return TOKENIZER_ERROR;
        }
      }
      if(!isdigit(info->ptr[i])) {
        DEBUG_PRINTF("get_next_token: error due to malformed number\n");
        return TOKENIZER_ERROR;
      }
    }
    DEBUG_PRINTF("get_next_token: error due to too long number\n");
    return TOKENIZER_ERROR;
  } else if(singlechar(info)) {
	  info->nextptr = info->ptr + 1;
    return singlechar(info);
  } else if(*info->ptr == '"') {
	  info->nextptr = info->ptr;
    do {
      ++info->nextptr;
    } while(*info->nextptr != '"');
    ++info->nextptr;
    return TOKENIZER_STRING;
  } else {
    for(kt = keywords; kt->keyword != NULL; ++kt) {
      if(strncmp(info->ptr, kt->keyword, strlen(kt->keyword)) == 0) {
    	  info->nextptr = info->ptr + strlen(kt->keyword);
        return kt->token;
      }
    }
  }

  if(*info->ptr >= 'a' && *info->ptr <= 'z') {
	  info->nextptr = info->ptr + 1;
    return TOKENIZER_VARIABLE;
  }


  return TOKENIZER_ERROR;
}
/*---------------------------------------------------------------------------*/
void
ubasic_tokenizer_goto(ubasic_tokenizer_info *info, const char *program)
{
	info->ptr = program;
  current_token = get_next_token(info);
}
/*---------------------------------------------------------------------------*/
void
ubasic_tokenizer_init(ubasic_tokenizer_info *info, const char *program)
{
	info->current_token = TOKENIZER_ERROR;
  ubasic_tokenizer_goto(info, program);
  current_token = get_next_token(info);
}
/*---------------------------------------------------------------------------*/
int
ubasic_tokenizer_token(ubasic_tokenizer_info *info)
{
  return current_token;
}
/*---------------------------------------------------------------------------*/
void
ubasic_tokenizer_next(ubasic_tokenizer_info *info)
{

  if(ubasic_tokenizer_finished(info)) {
    return;
  }

  DEBUG_PRINTF("tokenizer_next: %p\n", nextptr);
  info->ptr = info->nextptr;

  while(*info->ptr == ' ') {
    ++info->ptr;
  }
  current_token = get_next_token(info);

  if(current_token == TOKENIZER_REM) {
      while(!(*info->nextptr == '\n' || ubasic_tokenizer_finished(info))) {
        ++info->nextptr;
      }
      if(*info->nextptr == '\n') {
        ++info->nextptr;
      }
      ubasic_tokenizer_next(info);
  }

  DEBUG_PRINTF("tokenizer_next: '%s' %d\n", info->ptr, current_token);
  return;
}
/*---------------------------------------------------------------------------*/
VARIABLE_TYPE
ubasic_tokenizer_num(ubasic_tokenizer_info *info)
{
  return atoi(info->ptr);
}
/*---------------------------------------------------------------------------*/
void
ubasic_tokenizer_string(ubasic_tokenizer_info *info, char *dest, int len)
{
  char *string_end;
  int string_len;

  if(ubasic_tokenizer_token(info) != TOKENIZER_STRING) {
    return;
  }
  string_end = strchr(info->ptr + 1, '"');
  if(string_end == NULL) {
    return;
  }
  string_len = string_end - info->ptr - 1;
  if(len < string_len) {
    string_len = len;
  }
  memcpy(dest, info->ptr + 1, string_len);
  dest[string_len] = 0;
}
/*---------------------------------------------------------------------------*/
void
ubasic_tokenizer_error_print(ubasic_tokenizer_info *info)
{
  DEBUG_PRINTF("tokenizer_error_print: '%s'\n", info->ptr);
}
/*---------------------------------------------------------------------------*/
int
ubasic_tokenizer_finished(ubasic_tokenizer_info *info)
{
  return *info->ptr == 0 || current_token == TOKENIZER_ENDOFINPUT;
}
/*---------------------------------------------------------------------------*/
int
ubasic_tokenizer_variable_num(ubasic_tokenizer_info *info)
{
  return *info->ptr - 'a';
}
/*---------------------------------------------------------------------------*/
char const *
ubasic_tokenizer_pos(ubasic_tokenizer_info *info)
{
    return info->ptr;
}
