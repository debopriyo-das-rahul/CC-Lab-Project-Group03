CC = gcc
BISON = bison
FLEX = flex

TARGET = bin/compiler

SRC = src/parser/parser.tab.c \
      src/lexer/lex.yy.c \
      src/ast/ast.c \
      src/symbol_table/symbol_table.c \
      src/semantic/semantic.c \
      src/tac/tac.c

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(SRC) -lfl -o $(TARGET)

# Step 1: Bison first -- generates parser.tab.c and parser.tab.h
src/parser/parser.tab.c: src/parser/parser.y
	$(BISON) -d -o src/parser/parser.tab.c src/parser/parser.y

# Step 2: Flex second -- lexer.l includes parser.tab.h, so it must
# come after the Bison step above
src/lexer/lex.yy.c: src/lexer/lexer.l src/parser/parser.tab.c
	$(FLEX) -o src/lexer/lex.yy.c src/lexer/lexer.l

.PHONY: all clean run

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f src/parser/parser.tab.c src/parser/parser.tab.h
	rm -f src/lexer/lex.yy.c
	rm -rf bin