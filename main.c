#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- AST ---

// lambda term has 3 types
// Type           Syntax              Example
// variable       <name>              x
// abstraction    \<param>. <body>    \x. x
// application    <func> <arg>        (\x.x) a
typedef enum { VAR, ABT, APP } TermType;

typedef struct Term Term;

struct Term {
  TermType type;
  union {
    char *var;
    struct {
      char *param;
      Term *body;
    } abt;
    struct {
      Term *func;
      Term *arg;
    } app;
  } as;
};

Term *var(const char *name) {
  Term *term = malloc(sizeof(Term));
  assert(term != NULL);
  term->type = VAR;
  term->as.var = strdup(name);
  return term;
}

Term *abt(const char *param, Term *body) {
  Term *term = malloc(sizeof(Term));
  assert(term != NULL);
  term->type = ABT;
  term->as.abt.param = strdup(param);
  term->as.abt.body = body;
  return term;
}

Term *app(Term *func, Term *arg) {
  Term *term = malloc(sizeof(Term));
  assert(term != NULL);
  term->type = APP;
  term->as.app.func = func;
  term->as.app.arg = arg;
  return term;
}

// TODO: use some kind of string builder
void print_term_rec(Term *term) {
  switch (term->type) {
  case VAR:
    printf("%s", term->as.var);
    break;
  case ABT:
    printf("\\%s. ", term->as.abt.param);
    print_term_rec(term->as.abt.body);
    break;
  case APP:
    printf("(");
    print_term_rec(term->as.app.func);
    printf(") ");
    print_term_rec(term->as.app.arg);
    break;
  default:
    assert(!"Term_Type");
  }
}

void print_term(Term *term) {
  print_term_rec(term);
  printf("\n");
}

// --- REDUCTION ---

// substitute all occurances of param in body with val
Term *substitute(const char *param, Term *body, Term *val) {
  switch (body->type) {
  case VAR: {
    if (strcmp(body->as.var, param) == 0) {
      return val; // might need to detach somehow, incase free later on
    }
    return body;
  }
  case ABT:
    // assume unique var names for now

    return abt(body->as.abt.param, substitute(param, body->as.abt.body, val));
  case APP:
    return app(substitute(param, body->as.app.func, val),
               substitute(param, body->as.app.arg, val));
  default:
    assert(!"Term_Type");
  }
}

// recursively reduce term once
Term *reduce(Term *term) {
  switch (term->type) {
  case VAR:
    return term;
  case ABT: {
    Term *body = reduce(term->as.abt.body);
    if (body != term->as.abt.body) { // check if body was reduced
      // return fun_var_name(expr_slot(expr).as.fun.arg, body);
    }
    return term;
  }
  case APP: {
    Term *func = reduce(term->as.app.func);
    if (func != term->as.app.func) { // check if func was reduced
      return app(func, term->as.app.arg);
    }

    if (func->type == ABT) { // if not reduced, apply the lambda
      return substitute(func->as.abt.param, func->as.abt.body,
                        term->as.app.arg);
    }

    Term *arg = reduce(term->as.app.arg);
    if (arg != term->as.app.arg) { // check if arg was reduced
      return app(func, arg);
    }

    return term; // cannot be reduced
  }
  default:
    assert(!"Term_Type");
  }
}

int main() {
  Term *I = abt("x", var("x")); // \x. x
  // Term *expr = app(abt("x", var("x")), var("y")); // (\x. x) y ==> y
  Term *TRUE = abt("t", abt("f", var("t"))); // (\t. (\f. t))
  // Term *expr = app(app(TRUE, var("a")), var("b")); // TRUE a b ==> a
  // Term *expr = app(app(TRUE, var("a")), app(I, var("b"))); // TRUE a b ==> a
  Term *expr = app(app(TRUE, app(I, var("a"))), var("b")); // TRUE a b ==> a
  print_term(expr);

  Term *old;
  for (;;) {
    old = expr;
    expr = reduce(expr);
    if (expr == old)
      break; // no reduction was made
    print_term(expr);
  }

  return 0;
}
