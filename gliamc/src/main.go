package main

import (
	"github.com/jackdelahunt/Liam/gliamc/src/compiler"
	"log"
)

const (
	Code = `
struct Person {
}

fn add() int {
	return main();
}

fn main() bool {
	return jack;
}
`
)

func main() {
	source := []rune(Code)
	tokenBuffer := compiler.NewLexer(source).Lex()

	ast, err := compiler.NewParser(source, tokenBuffer).Parse()
	if err != nil {
		log.Fatal(err.Error())
	}

	ast.PrintAST()
}
