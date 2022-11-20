package src

import "fmt"

type Expression interface {
	codegen() string
}

type TypeExpression interface {
	codegen() string
}

type BinaryExpression struct {
	Left  Expression
	Op    OperatorType
	Right Expression
}

type NumberLiteralExpression struct {
	Token Token
	Value float64
}

type StringLiteralExpression struct {
	Token Token
}

type BoolLiteralExpression struct {
	Token Token
}

type IdentifierLiteralExpression struct {
	Identifier Token
}

type IdentifierTypeExpression struct {
	Identifier Token
}

func (expr BinaryExpression) Codegen() string {
	output := expr.Left.codegen()

	if expr.Op == OP_PLUS {
		output += " + "
	} else if expr.Op == OP_MINUS {
		output += " - "
	} else if expr.Op == OP_MULT {
		output += " * "
	} else if expr.Op == OP_DIV {
		output += " / "
	}

	return output + expr.Right.codegen()
}

func (expr NumberLiteralExpression) codegen() string {
	return fmt.Sprintf("%f", expr.Value)
}
