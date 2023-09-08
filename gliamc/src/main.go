package main

import (
	"github.com/jackdelahunt/Liam/gliamc/src/compiler"
	"log"
)

const (
	Code = `
fn add() int {
	if main() {
		return 0;
	}
}

fn main() bool {
	return 5 + 5;
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

	typedAST, err := compiler.NewTypeChecker(ast).TypeCheck()
	if err != nil {
		log.Fatal(err.Error())
	}

	builder := compiler.NewIRBuilder(typedAST)
	err = builder.BuildIR()
	if err != nil {
		log.Fatal(err.Error())
	}

	builder.PrintIR()
}
