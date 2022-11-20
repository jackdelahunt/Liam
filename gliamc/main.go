package main

import (
	"fmt"
	liamc "github.com/jackdelahunt/Liam/gliamc/src"
)

func main() {
	expr := liamc.BinaryExpression{
		Left: liamc.NumberLiteralExpression{
			Value: 100,
		},
		Op: liamc.OP_PLUS,
		Right: liamc.NumberLiteralExpression{
			Value: 200,
		},
	}

	fmt.Println(expr.Codegen())
}
