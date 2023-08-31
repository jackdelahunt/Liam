package main

import (
	"github.com/jackdelahunt/Liam/gliamc/src/compiler"
	"log"
)

const (
	Code = `
fn main() bool {
	return 5;
}
`
)

func main() {
	//err := liamCLI.StartCLI()
	//if err != nil {
	//	fmt.Println(err.Error())
	//}
	source := []rune(Code)
	tokenBuffer := compiler.NewLexer(source).Lex()

	ast, err := compiler.NewParser(source, tokenBuffer).Parse()
	if err != nil {
		log.Fatal(err.Error())
	}

	builder := compiler.NewIRBuilder(ast)
	err = builder.BuildIR()
	if err != nil {
		log.Fatal(err.Error())
	}

	builder.PrintIR()
}
