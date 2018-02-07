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
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "vartype.h"

enum {
  TOKENIZER_ERROR,
  TOKENIZER_ENDOFINPUT,
  TOKENIZER_NUMBER,
  TOKENIZER_STRING,
  TOKENIZER_VARIABLE,
  TOKENIZER_LET,
  TOKENIZER_PRINT,
  TOKENIZER_IF,
  TOKENIZER_THEN,
  TOKENIZER_ELSE,
  TOKENIZER_FOR,
  TOKENIZER_TO,
  TOKENIZER_NEXT,
  TOKENIZER_GOTO,
  TOKENIZER_GOSUB,
  TOKENIZER_RETURN,
  TOKENIZER_CALL,
  TOKENIZER_REM,
  TOKENIZER_PEEK,
  TOKENIZER_POKE,
  TOKENIZER_END,
  TOKENIZER_COMMA,
  TOKENIZER_SEMICOLON,
  TOKENIZER_PLUS,
  TOKENIZER_MINUS,
  TOKENIZER_AND,
  TOKENIZER_OR,
  TOKENIZER_ASTR,
  TOKENIZER_SLASH,
  TOKENIZER_MOD,
  TOKENIZER_HASH,
  TOKENIZER_LEFTPAREN,
  TOKENIZER_RIGHTPAREN,
  TOKENIZER_LT,
  TOKENIZER_GT,
  TOKENIZER_EQ,
  TOKENIZER_CR,
};

typedef struct {
	char const *ptr;
	char const *nextptr;
	int current_token;
} ubasic_tokenizer_info;

void ubasic_tokenizer_goto(ubasic_tokenizer_info *info, const char *program);
void ubasic_tokenizer_init(ubasic_tokenizer_info *info, const char *program);
void ubasic_tokenizer_next(ubasic_tokenizer_info *info);
int ubasic_tokenizer_token(ubasic_tokenizer_info *info);
VARIABLE_TYPE ubasic_tokenizer_num(ubasic_tokenizer_info *info);
int ubasic_tokenizer_variable_num(ubasic_tokenizer_info *info);
void ubasic_tokenizer_string(ubasic_tokenizer_info *info, char *dest, int len);

int ubasic_tokenizer_finished(ubasic_tokenizer_info *info);
void ubasic_tokenizer_error_print(ubasic_tokenizer_info *info);

char const *ubasic_tokenizer_pos(ubasic_tokenizer_info *info);

#endif /* __TOKENIZER_H__ */
