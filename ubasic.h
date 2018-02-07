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
#ifndef __UBASIC_H__
#define __UBASIC_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "vartype.h"
#include "tokenizer.h"

#define MAX_STRINGLEN 40
#define MAX_GOSUB_STACK_DEPTH 10
#define MAX_FOR_STACK_DEPTH 4
#define MAX_VARNUM 26


typedef VARIABLE_TYPE (*peek_func)(VARIABLE_TYPE, void *);
typedef VARIABLE_TYPE (*usr_func)(VARIABLE_TYPE, void *);
typedef VARIABLE_TYPE (*input_func)(VARIABLE_TYPE, void *);
typedef void (*poke_func)(VARIABLE_TYPE, VARIABLE_TYPE, void *);

typedef void (*begin_func)(void *);
typedef void (*handle_num_func)(VARIABLE_TYPE, void *);
typedef void (*handle_string_func)(const char *, void *);
typedef void (*end_func)(void *);

struct ubasic_for_state {
  int line_after_for;
  int for_variable;
  int to;
};

struct ubasic_line_index {
  int line_number;
  char const *program_text_position;
  struct ubasic_line_index *next;
};


typedef struct {
	void *app_context;

	char const *program_ptr;
	char string[MAX_STRINGLEN];

	int gosub_stack[MAX_GOSUB_STACK_DEPTH];
	int gosub_stack_ptr;

	struct ubasic_for_state for_stack[MAX_FOR_STACK_DEPTH];
	int for_stack_ptr;

	struct ubasic_line_index *line_index_head;
	struct ubasic_line_index *line_index_current;

	VARIABLE_TYPE variables[MAX_VARNUM];

	int ended;

	peek_func peek_function;
	poke_func poke_function;
	usr_func  usr_function;
	input_func input_function;

	begin_func print_begin_function;
	handle_num_func print_num_function;
	handle_string_func print_string_function;
	end_func print_end_function;

	ubasic_tokenizer_info tokenizer_info;
} ubasic_info;


void ubasic_init(ubasic_info *info, const char *program);
void ubasic_run(ubasic_info *info);
int ubasic_finished(ubasic_info *info);

VARIABLE_TYPE ubasic_get_variable(ubasic_info *info, int varnum);
void ubasic_set_variable(ubasic_info *info, int varum, VARIABLE_TYPE value);


#ifdef __cplusplus
}
#endif

#endif /* __UBASIC_H__ */
