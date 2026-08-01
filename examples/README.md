# Examples

Representative sample programs showing the core language features
(Project Manual, Section 5). These are meant to be run individually to
demonstrate the compiler working end-to-end, separate from the
pass/fail regression cases under `tests/`.

| File | Demonstrates |
|---|---|
| `01_variables_and_arithmetic.mini` | `int`/`float`/`bool` declarations, arithmetic expressions, operator precedence, `print` |
| `02_control_flow.mini` | `while` loop, relational/logical operators, `if`-`else` |
| `03_nested_scope.mini` | Nested blocks (`{ ... }`) and variable shadowing across scopes |

Each `.mini` file has a matching `.out` file holding the actual
output captured from the compiler (AST, symbol table, and TAC), for
quick reference without having to rebuild.

## Running an example

```
make
./bin/compiler < examples/01_variables_and_arithmetic.mini
```
