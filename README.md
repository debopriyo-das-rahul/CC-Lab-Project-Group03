# Mini Language Compiler

A compiler front-end for the mini programming language specified in
the Compiler Construction Lab Project Manual, Section 5. Implements
lexical analysis, syntax analysis, AST construction, semantic
analysis, and Three Address Code (TAC) generation using Flex and
Bison.

## Group 03


| Name | Student ID | Contribution |
|:----:|:----------:|:------------:|
|Debopriyo Das Rahul|231-115-126|lexer,parser,Makefile,examples,tests,README,test running script,project report|
|Amit Banik|231-115-144|ast,semantic,examples,project report,presentation slides|
|Jasmin Amin Tarafder|222-115-016|symbol table,tac,examples,project report,presentation slides|
## Features

- **Lexical analysis** (`src/lexer/lexer.l`): keywords (`int`,
  `float`, `bool`, `if`, `else`, `while`, `print`, `true`, `false`),
  identifiers, integer/float literals, all arithmetic/relational/
  logical operators, `//` and `/* */` comments, and line-numbered
  lexical error reporting for invalid tokens.
- **Syntax analysis** (`src/parser/parser.y`): full CFG for the
  language (see `docs/grammar.md`) implemented in Bison, with
  panic-mode error recovery (`error ;`) so a single syntax error
  doesn't abort the whole parse.
- **Abstract Syntax Tree** (`src/ast/`): one node per language
  construct, printable via `print_tree()`.
- **Symbol table** (`src/symbol_table/`): name/type/scope/declaration
  line per entry, with nested-scope support via
  `enter_scope()`/`exit_scope()`.
- **Semantic analysis** (`src/semantic/`): detects undeclared
  variable use, redeclaration, scope violations, type mismatches,
  invalid assignments, and invalid expressions (Project Manual,
  Section 4.5).
- **Intermediate code generation** (`src/tac/`): TAC for arithmetic,
  relational, and logical expressions, `if`/`if-else`/`while` via
  labels and jumps, and `print` statements.

## Project Structure

```
.
├── docs/               Grammar spec, architecture diagram, project report
├── examples/           Representative sample programs (see examples/README.md)
├── src/
│   ├── lexer/          Flex specification (lexer.l)
│   ├── parser/         Bison grammar (parser.y), also owns main()
│   ├── ast/            AST node definitions and printing
│   ├── symbol_table/   Symbol table with nested-scope support
│   ├── semantic/       Semantic analysis / type checking
│   └── tac/            Three Address Code generation
├── tests/              Valid and invalid test programs and outputs
├── Makefile
└── run_tests.sh        Builds and runs every test in tests/
```

## Build Instructions

Requires `gcc`, `bison`, and `flex`.

```
make
```

This runs Bison on `parser.y` first (generating `parser.tab.c/.h`),
then Flex on `lexer.l` (generating `lex.yy.c`), then compiles
everything into `bin/compiler`.

To clean generated files:

```
make clean
```

## Execution Instructions

The compiler reads a source program from standard input:

```
./bin/compiler < examples/01_variables_and_arithmetic.mini
```

Output is printed in four stages: the parse status, the AST, the
symbol table plus any semantic errors, and (only if there are no
semantic errors) the generated TAC.

## Running the Test Suite

```
./run_tests.sh
```

Builds the project and runs the compiler against every program under
`tests/`, covering valid compilation, lexical errors, syntax errors,
and each semantic error category.

## Documentation

- `docs/project report.pdf` — project report
- `examples/README.md` — index of sample programs and what each demonstrates