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

#include "ubasic.h"
#include "tokenizer.h"

// TODO need to abstract out printf
#include <stdio.h> /* printf() */
// TODO need to abstract out exit
#include <stdlib.h> /* exit() */

//static char const *program_ptr;
//static char string[MAX_STRINGLEN];
//
//static int gosub_stack[MAX_GOSUB_STACK_DEPTH];
//static int gosub_stack_ptr;
//
//static struct ubasic_for_state for_stack[MAX_FOR_STACK_DEPTH];
//static int for_stack_ptr;
//
//struct ubasic_line_index *line_index_head = NULL;
//struct ubasic_line_index *line_index_current = NULL;
//
//static VARIABLE_TYPE variables[MAX_VARNUM];
//
//static int ended;
//
//static VARIABLE_TYPE expr(ubasic_info *info);
//static void line_statement(ubasic_info *info);
//static void statement(ubasic_info *info);
//static void index_free(ubasic_info *info);

//peek_func peek_function = NULL;
//poke_func poke_function = NULL;

/*---------------------------------------------------------------------------*/
static void accept(ubasic_info *info, int token);
static int varfactor(ubasic_info *info);
static int factor(ubasic_info *info);
static int term(ubasic_info *info);
static VARIABLE_TYPE expr(ubasic_info *info);
static int relation(ubasic_info *info);
static void index_free(ubasic_info *info);
static char const* index_find(ubasic_info *info, int linenum);
static void index_add(ubasic_info *info, int linenum, char const* sourcepos);
static void jump_linenum_slow(ubasic_info *info, int linenum);
static void jump_linenum(ubasic_info *info, int linenum);
static void goto_statement(ubasic_info *info);
static void print_statement(ubasic_info *info);
static void if_statement(ubasic_info *info);
static void let_statement(ubasic_info *info);
static void gosub_statement(ubasic_info *info);
static void return_statement(ubasic_info *info);
static void next_statement(ubasic_info *info);
static void for_statement(ubasic_info *info);
static void input_statement(ubasic_info *info);
static void peek_statement(ubasic_info *info);
static void poke_statement(ubasic_info *info);
static void end_statement(ubasic_info *info);
static void statement(ubasic_info *info);
static void line_statement(ubasic_info *info);

static void print_begin(void *context);
static void print_num(VARIABLE_TYPE num, void *context);
static void print_string(const char *str, void *context);
static void print_separator(const char sep, void *context);
static void print_end(void *context);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
void
ubasic_init(ubasic_info *info, const char *program)
{
  info->program_ptr = program;
  info->for_stack_ptr = 0;
  info->gosub_stack_ptr = 0;

  info->line_index_head = NULL;
  info->line_index_current = NULL;

  info->peek_function = NULL;
  info->poke_function = NULL;
  info->input_function = NULL;

  info->print_begin_function = print_begin;
  info->print_num_function = print_num;
  info->print_string_function = print_string;
  info->print_separator_function = print_separator;
  info->print_end_function = print_end;

  info->user_begin_function = NULL;
  info->user_num_function = NULL;
  info->user_separator_function = NULL;
  info->user_string_function = NULL;
  info->user_end_function = NULL;

  index_free(info);
  ubasic_tokenizer_init(&info->tokenizer_info, program);

  info->ended = 0;
}
/*---------------------------------------------------------------------------*/
static void
accept(ubasic_info *info, int token)
{
  if(token != ubasic_tokenizer_token(&info->tokenizer_info)) {
    DEBUG_PRINTF("Token not what was expected (expected %d, got %d)\n",
                token, ubasic_tokenizer_token(&info->tokenizer_info));
    ubasic_tokenizer_error_print(&info->tokenizer_info);
    exit(1);
  }
  DEBUG_PRINTF("Expected %d, got it\n", token);
  ubasic_tokenizer_next(&info->tokenizer_info);
}
/*---------------------------------------------------------------------------*/
static int
varfactor(ubasic_info *info)
{
  int r;
  DEBUG_PRINTF("varfactor: obtaining %d from variable %d\n", variables[ubasic_tokenizer_variable_num()], ubasic_tokenizer_variable_num());
  r = ubasic_get_variable(info, ubasic_tokenizer_variable_num(&info->tokenizer_info));
  accept(info, TOKENIZER_VARIABLE);
  return r;
}
/*---------------------------------------------------------------------------*/
static int
factor(ubasic_info *info)
{
  int r;

  DEBUG_PRINTF("factor: token %d\n", ubasic_tokenizer_token(&info->tokenizer_info));
  switch(ubasic_tokenizer_token(&info->tokenizer_info)) {
  case TOKENIZER_NUMBER:
    r = ubasic_tokenizer_num(&info->tokenizer_info);
    DEBUG_PRINTF("factor: number %d\n", r);
    accept(info, TOKENIZER_NUMBER);
    break;
  case TOKENIZER_LEFTPAREN:
    accept(info, TOKENIZER_LEFTPAREN);
    r = expr(info);
    accept(info, TOKENIZER_RIGHTPAREN);
    break;
  default:
    r = varfactor(info);
    break;
  }
  return r;
}
/*---------------------------------------------------------------------------*/
static int
term(ubasic_info *info)
{
  int f1, f2;
  int op;

  f1 = factor(info);
  op = ubasic_tokenizer_token(&info->tokenizer_info);
  DEBUG_PRINTF("term: token %d\n", op);
  while(op == TOKENIZER_ASTR ||
       op == TOKENIZER_SLASH ||
       op == TOKENIZER_MOD) {
    ubasic_tokenizer_next(&info->tokenizer_info);
    f2 = factor(info);
    DEBUG_PRINTF("term: %d %d %d\n", f1, op, f2);
    switch(op) {
    case TOKENIZER_ASTR:
      f1 = f1 * f2;
      break;
    case TOKENIZER_SLASH:
      f1 = f1 / f2;
      break;
    case TOKENIZER_MOD:
      f1 = f1 % f2;
      break;
    }
    op = ubasic_tokenizer_token(&info->tokenizer_info);
  }
  DEBUG_PRINTF("term: %d\n", f1);
  return f1;
}
/*---------------------------------------------------------------------------*/
static VARIABLE_TYPE
expr(ubasic_info *info)
{
  int t1, t2;
  int op;

  t1 = term(info);
  op = ubasic_tokenizer_token(&info->tokenizer_info);
  DEBUG_PRINTF("expr: token %d\n", op);
  while(op == TOKENIZER_PLUS ||
       op == TOKENIZER_MINUS ||
       op == TOKENIZER_AND ||
       op == TOKENIZER_OR) {
    ubasic_tokenizer_next(&info->tokenizer_info);
    t2 = term(info);
    DEBUG_PRINTF("expr: %d %d %d\n", t1, op, t2);
    switch(op) {
    case TOKENIZER_PLUS:
      t1 = t1 + t2;
      break;
    case TOKENIZER_MINUS:
      t1 = t1 - t2;
      break;
    case TOKENIZER_AND:
      t1 = t1 & t2;
      break;
    case TOKENIZER_OR:
      t1 = t1 | t2;
      break;
    }
    op = ubasic_tokenizer_token(&info->tokenizer_info);
  }
  DEBUG_PRINTF("expr: %d\n", t1);
  return t1;
}
/*---------------------------------------------------------------------------*/
static int
relation(ubasic_info *info)
{
  int r1, r2;
  int op;

  r1 = expr(info);
  op = ubasic_tokenizer_token(&info->tokenizer_info);
  DEBUG_PRINTF("relation: token %d\n", op);
  while(op == TOKENIZER_LT ||
       op == TOKENIZER_GT ||
       op == TOKENIZER_EQ) {
    ubasic_tokenizer_next(&info->tokenizer_info);
    r2 = expr(info);
    DEBUG_PRINTF("relation: %d %d %d\n", r1, op, r2);
    switch(op) {
    case TOKENIZER_LT:
      r1 = r1 < r2;
      break;
    case TOKENIZER_GT:
      r1 = r1 > r2;
      break;
    case TOKENIZER_EQ:
      r1 = r1 == r2;
      break;
    }
    op = ubasic_tokenizer_token(&info->tokenizer_info);
  }
  return r1;
}
/*---------------------------------------------------------------------------*/
static void
index_free(ubasic_info *info) {
  if(info->line_index_head != NULL) {
    info->line_index_current = info->line_index_head;
    do {
      DEBUG_PRINTF("Freeing index for line %d.\n", line_index_current);
      info->line_index_head = info->line_index_current;
      info->line_index_current = info->line_index_current->next;
      free(info->line_index_head);
    } while (info->line_index_current != NULL);
    info->line_index_head = NULL;
  }
}
/*---------------------------------------------------------------------------*/
static char const*
index_find(ubasic_info *info, int linenum) {
  struct ubasic_line_index *lidx;
  lidx = info->line_index_head;

  #if DEBUG
  int step = 0;
  #endif

  while(lidx != NULL && lidx->line_number != linenum) {
    lidx = lidx->next;

    #if DEBUG
    if(lidx != NULL) {
      DEBUG_PRINTF("index_find: Step %3d. Found index for line %d: %d.\n",
                   step, lidx->line_number,
                   lidx->program_text_position);
    }
    step++;
    #endif
  }
  if(lidx != NULL && lidx->line_number == linenum) {
    DEBUG_PRINTF("index_find: Returning index for line %d.\n", linenum);
    return lidx->program_text_position;
  }
  DEBUG_PRINTF("index_find: Returning NULL.\n", linenum);
  return NULL;
}
/*---------------------------------------------------------------------------*/
static void
index_add(ubasic_info *info, int linenum, char const* sourcepos) {
  if(info->line_index_head != NULL && index_find(info, linenum)) {
    return;
  }

  struct ubasic_line_index *new_lidx;

  new_lidx = malloc(sizeof(struct ubasic_line_index));
  new_lidx->line_number = linenum;
  new_lidx->program_text_position = sourcepos;
  new_lidx->next = NULL;

  if(info->line_index_head != NULL) {
    info->line_index_current->next = new_lidx;
    info->line_index_current = info->line_index_current->next;
  } else {
    info->line_index_current = new_lidx;
    info->line_index_head = info->line_index_current;
  }
  DEBUG_PRINTF("index_add: Adding index for line %d: %d.\n", linenum,
               sourcepos);
}
/*---------------------------------------------------------------------------*/
static void
jump_linenum_slow(ubasic_info *info, int linenum)
{
  ubasic_tokenizer_init(&info->tokenizer_info, info->program_ptr);
  while(ubasic_tokenizer_num(&info->tokenizer_info) != linenum) {
    do {
      do {
        ubasic_tokenizer_next(&info->tokenizer_info);
      } while(ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_CR &&
          ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_ENDOFINPUT);
      if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_CR) {
        ubasic_tokenizer_next(&info->tokenizer_info);
      }
    } while(ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_NUMBER);
    DEBUG_PRINTF("jump_linenum_slow: Found line %d\n", ubasic_tokenizer_num());
  }
}
/*---------------------------------------------------------------------------*/
static void
jump_linenum(ubasic_info *info, int linenum)
{
  char const* pos = index_find(info, linenum);
  if(pos != NULL) {
    DEBUG_PRINTF("jump_linenum: Going to line %d.\n", linenum);
    ubasic_tokenizer_goto(&info->tokenizer_info, pos);
  } else {
    /* We'll try to find a yet-unindexed line to jump to. */
    DEBUG_PRINTF("jump_linenum: Calling jump_linenum_slow.\n", linenum);
    jump_linenum_slow(info, linenum);
  }
}
/*---------------------------------------------------------------------------*/
static void
goto_statement(ubasic_info *info)
{
  accept(info, TOKENIZER_GOTO);
  jump_linenum(info, ubasic_tokenizer_num(&info->tokenizer_info));
}
/*---------------------------------------------------------------------------*/
static void
print_statement(ubasic_info *info)
{
  accept(info, TOKENIZER_PRINT);

  if(info->print_begin_function != NULL) {
	  info->print_begin_function(info->app_context);
  }

  do {
    DEBUG_PRINTF("Print loop\n");
    if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_STRING) {
      ubasic_tokenizer_string(&info->tokenizer_info, info->string, sizeof(info->string));

      if(info->print_string_function != NULL) {
    	  info->print_string_function(info->string, info->app_context);
      }

      ubasic_tokenizer_next(&info->tokenizer_info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_COMMA) {

    	if(info->print_separator_function != NULL) {
    		info->print_separator_function(',', info->app_context);
    	}

      ubasic_tokenizer_next(&info->tokenizer_info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_SEMICOLON) {
      ubasic_tokenizer_next(&info->tokenizer_info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_VARIABLE ||
          ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_NUMBER) {

    	if(info->print_num_function != NULL) {
    		info->print_num_function(expr(info), info->app_context);
    	}
    } else {
      break;
    }
  } while(ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_CR &&
      ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_ENDOFINPUT);

  if(info->print_end_function != NULL) {
	  info->print_end_function(info->app_context);
  }
  DEBUG_PRINTF("End of print\n");
  ubasic_tokenizer_next(&info->tokenizer_info);
}
/*---------------------------------------------------------------------------*/
static void
if_statement(ubasic_info *info)
{
  int r;

  accept(info, TOKENIZER_IF);

  r = relation(info);
  DEBUG_PRINTF("if_statement: relation %d\n", r);
  accept(info, TOKENIZER_THEN);
  if(r) {
    statement(info);
  } else {
    do {
      ubasic_tokenizer_next(&info->tokenizer_info);
    } while(ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_ELSE &&
        ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_CR &&
        ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_ENDOFINPUT);
    if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_ELSE) {
      ubasic_tokenizer_next(&info->tokenizer_info);
      statement(info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_CR) {
      ubasic_tokenizer_next(&info->tokenizer_info);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
let_statement(ubasic_info *info)
{
  int var;

  var = ubasic_tokenizer_variable_num(&info->tokenizer_info);

  accept(info, TOKENIZER_VARIABLE);
  accept(info, TOKENIZER_EQ);
  ubasic_set_variable(info, var, expr(info));
  DEBUG_PRINTF("let_statement: assign %d to %d\n", variables[var], var);
  accept(info, TOKENIZER_CR);

}
/*---------------------------------------------------------------------------*/
static void
gosub_statement(ubasic_info *info)
{
  int linenum;
  accept(info, TOKENIZER_GOSUB);
  linenum = ubasic_tokenizer_num(&info->tokenizer_info);
  accept(info, TOKENIZER_NUMBER);
  accept(info, TOKENIZER_CR);
  if(info->gosub_stack_ptr < MAX_GOSUB_STACK_DEPTH) {
    info->gosub_stack[info->gosub_stack_ptr] = ubasic_tokenizer_num(&info->tokenizer_info);
    info->gosub_stack_ptr++;
    jump_linenum(info, linenum);
  } else {
    DEBUG_PRINTF("gosub_statement: gosub stack exhausted\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
return_statement(ubasic_info *info)
{
  accept(info, TOKENIZER_RETURN);
  if(info->gosub_stack_ptr > 0) {
    info->gosub_stack_ptr--;
    jump_linenum(info, info->gosub_stack[info->gosub_stack_ptr]);
  } else {
    DEBUG_PRINTF("return_statement: non-matching return\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
next_statement(ubasic_info *info)
{
  int var;

  accept(info, TOKENIZER_NEXT);
  var = ubasic_tokenizer_variable_num(&info->tokenizer_info);
  accept(info, TOKENIZER_VARIABLE);
  if(info->for_stack_ptr > 0 &&
     var == info->for_stack[info->for_stack_ptr - 1].for_variable) {
    ubasic_set_variable(info, var,
                       ubasic_get_variable(info, var) + 1);
    if(ubasic_get_variable(info, var) <= info->for_stack[info->for_stack_ptr - 1].to) {
      jump_linenum(info, info->for_stack[info->for_stack_ptr - 1].line_after_for);
    } else {
      info->for_stack_ptr--;
      accept(info, TOKENIZER_CR);
    }
  } else {
    DEBUG_PRINTF("next_statement: non-matching next (expected %d, found %d)\n", for_stack[for_stack_ptr - 1].for_variable, var);
    accept(info, TOKENIZER_CR);
  }

}
/*---------------------------------------------------------------------------*/
static void
for_statement(ubasic_info *info)
{
  int for_variable, to;

  accept(info, TOKENIZER_FOR);
  for_variable = ubasic_tokenizer_variable_num(&info->tokenizer_info);
  accept(info, TOKENIZER_VARIABLE);
  accept(info, TOKENIZER_EQ);
  ubasic_set_variable(info, for_variable, expr(info));
  accept(info, TOKENIZER_TO);
  to = expr(info);
  accept(info, TOKENIZER_CR);

  if(info->for_stack_ptr < MAX_FOR_STACK_DEPTH) {
    info->for_stack[info->for_stack_ptr].line_after_for = ubasic_tokenizer_num(&info->tokenizer_info);
    info->for_stack[info->for_stack_ptr].for_variable = for_variable;
    info->for_stack[info->for_stack_ptr].to = to;
    DEBUG_PRINTF("for_statement: new for, var %d to %d\n",
                for_stack[for_stack_ptr].for_variable,
                for_stack[for_stack_ptr].to);

    info->for_stack_ptr++;
  } else {
    DEBUG_PRINTF("for_statement: for stack depth exceeded\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
input_statement(ubasic_info *info)
{
  VARIABLE_TYPE usr_value;
  int var;

  accept(info, TOKENIZER_INPUT);
  usr_value = expr(info);
  accept(info, TOKENIZER_COMMA);
  var = ubasic_tokenizer_variable_num(&info->tokenizer_info);
  accept(info, TOKENIZER_VARIABLE);
  accept(info, TOKENIZER_CR);

  if(info->input_function != NULL) {
    ubasic_set_variable(info, var, info->input_function(usr_value, info->app_context));
  }
}
/*---------------------------------------------------------------------------*/
static void
user_statement(ubasic_info *info)
{
  accept(info, TOKENIZER_USER);

  info->user_begin_function(info->app_context);

  do {
    DEBUG_PRINTF("User loop\n");
    if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_STRING) {
      ubasic_tokenizer_string(&info->tokenizer_info, info->string, sizeof(info->string));

      if(info->user_string_function != NULL) {
        info->user_string_function(info->string, info->app_context);
      }

      ubasic_tokenizer_next(&info->tokenizer_info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_COMMA) {
      if(info->user_separator_function != NULL) {
        info->user_separator_function(',', info->app_context);
      }

      ubasic_tokenizer_next(&info->tokenizer_info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_SEMICOLON) {
      if(info->user_separator_function != NULL) {
        info->user_separator_function(';', info->app_context);
      }

      ubasic_tokenizer_next(&info->tokenizer_info);
    } else if(ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_VARIABLE ||
      ubasic_tokenizer_token(&info->tokenizer_info) == TOKENIZER_NUMBER) {

      if(info->user_separator_function != NULL) {
        info->user_num_function(expr(info), info->app_context);
      }
    } else {
      break;
    }
  } while(ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_CR &&
          ubasic_tokenizer_token(&info->tokenizer_info) != TOKENIZER_ENDOFINPUT);

  if(info->user_end_function != NULL) {
	  info->user_end_function(info->app_context);
  }

  DEBUG_PRINTF("End of User\n");
  ubasic_tokenizer_next(&info->tokenizer_info);
}
/*---------------------------------------------------------------------------*/
static void
peek_statement(ubasic_info *info)
{
  VARIABLE_TYPE peek_addr;
  int var;

  accept(info, TOKENIZER_PEEK);
  peek_addr = expr(info);
  accept(info, TOKENIZER_COMMA);
  var = ubasic_tokenizer_variable_num(&info->tokenizer_info);
  accept(info, TOKENIZER_VARIABLE);
  accept(info, TOKENIZER_CR);

  if(info->peek_function != NULL) {
    ubasic_set_variable(info, var, info->peek_function(peek_addr, info->app_context));
  }
}
/*---------------------------------------------------------------------------*/
static void
poke_statement(ubasic_info *info)
{
  VARIABLE_TYPE poke_addr;
  VARIABLE_TYPE value;

  accept(info, TOKENIZER_POKE);
  poke_addr = expr(info);
  accept(info, TOKENIZER_COMMA);
  value = expr(info);
  accept(info, TOKENIZER_CR);

  if(info->poke_function != NULL) {
    info->poke_function(poke_addr, value, info->app_context);
  }
}
/*---------------------------------------------------------------------------*/
static void
end_statement(ubasic_info *info)
{
  accept(info, TOKENIZER_END);
  info->ended = 1;
}
/*---------------------------------------------------------------------------*/
static void
statement(ubasic_info *info)
{
  int token;

  token = ubasic_tokenizer_token(&info->tokenizer_info);

  switch(token) {
  case TOKENIZER_PRINT:
    print_statement(info);
    break;
  case TOKENIZER_IF:
    if_statement(info);
    break;
  case TOKENIZER_GOTO:
    goto_statement(info);
    break;
  case TOKENIZER_GOSUB:
    gosub_statement(info);
    break;
  case TOKENIZER_RETURN:
    return_statement(info);
    break;
  case TOKENIZER_FOR:
    for_statement(info);
    break;
  case TOKENIZER_INPUT:
    input_statement(info);
    break;
  case TOKENIZER_USER:
    user_statement(info);
    break;
  case TOKENIZER_PEEK:
    peek_statement(info);
    break;
  case TOKENIZER_POKE:
    poke_statement(info);
    break;
  case TOKENIZER_NEXT:
    next_statement(info);
    break;
  case TOKENIZER_END:
    end_statement(info);
    break;
  case TOKENIZER_LET:
    accept(info, TOKENIZER_LET);
    /* Fall through. */
  case TOKENIZER_VARIABLE:
    let_statement(info);
    break;
  default:
    DEBUG_PRINTF("ubasic.c: statement(): not implemented %d\n", token);
    exit(1);
  }
}
/*---------------------------------------------------------------------------*/
static void
line_statement(ubasic_info *info)
{
  DEBUG_PRINTF("----------- Line number %d ---------\n", ubasic_tokenizer_num());
  index_add(info, ubasic_tokenizer_num(&info->tokenizer_info), ubasic_tokenizer_pos(&info->tokenizer_info));
  accept(info, TOKENIZER_NUMBER);
  statement(info);
  return;
}
/*---------------------------------------------------------------------------*/
void
ubasic_run(ubasic_info *info)
{
  if(ubasic_tokenizer_finished(&info->tokenizer_info)) {
    DEBUG_PRINTF("uBASIC program finished\n");
    return;
  }

  line_statement(info);
}
/*---------------------------------------------------------------------------*/
int
ubasic_finished(ubasic_info *info)
{
  return info->ended || ubasic_tokenizer_finished(&info->tokenizer_info);
}
/*---------------------------------------------------------------------------*/
void
ubasic_set_variable(ubasic_info *info, int varnum, VARIABLE_TYPE value)
{
  if(varnum >= 0 && varnum <= MAX_VARNUM) {
    info->variables[varnum] = value;
  }
}
/*---------------------------------------------------------------------------*/
VARIABLE_TYPE
ubasic_get_variable(ubasic_info *info, int varnum)
{
  if(varnum >= 0 && varnum <= MAX_VARNUM) {
    return info->variables[varnum];
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
print_begin(void *context)
{
   // Nothing
}
/*---------------------------------------------------------------------------*/
static void
print_num(VARIABLE_TYPE num, void *context)
{
  printf("%d", num);
}
/*---------------------------------------------------------------------------*/
static void
print_string(const char *str, void *context)
{
  printf("%s", str);
}
/*---------------------------------------------------------------------------*/
static void
print_separator(const char sep, void *context)
{
  printf("%s", " ");
}
/*---------------------------------------------------------------------------*/
static void
print_end(void *context)
{
  printf("\n");
}
/*---------------------------------------------------------------------------*/
