package compiler

import (
	"fmt"
	"log"
)

type InstructionType = uint8

const UNUSED_SLOT_FLAG uint = 0x_cccc_cccc_cccc_cccc

const (
	ILoadNumberLiteral InstructionType = iota
	IBinaryAdd

	// :::::::::::::::::::::::::::::::::::
	// above expressions, below statements
	// :::::::::::::::::::::::::::::::::::

	IFnStart
	IFnEnd
	IReturn
)

type Instruction struct {
	instructionType InstructionType
	highSlot        uint
	lowSlot         uint
}

type IRBuilder struct {
	TypedAST TypedAST
	ByteCode []Instruction
}

func NewIRBuilder(typedAST TypedAST) *IRBuilder {
	return &IRBuilder{
		TypedAST: typedAST,
		ByteCode: make([]Instruction, 0),
	}
}

func (self *IRBuilder) BuildIR() error {
	for index, fnTypeInfo := range self.TypedAST.FnTypeInfos {
		self.GenerateBytecodeForFn(uint(index), fnTypeInfo)
	}
	return nil
}

func (self *IRBuilder) GenerateBytecodeForFn(index uint, fnTypeInfo *FnTypeInfo) {
	start := self.BuildFnStartInstruction(fnTypeInfo.Identifier, index)
	end := self.BuildFnEndInstruction()

	self.AddInstructionToBytecode(start)

	for _, statement := range self.TypedAST.FnStatements[index].Body {
		self.GenerateBytecodeForStatement(statement)
	}

	self.AddInstructionToBytecode(end)
}

func (self *IRBuilder) GenerateBytecodeForStatement(statement Statement) uint {
	switch statement.(type) {
	case *ReturnStatement:
		return self.GenerateBytecodeForReturn(statement.(*ReturnStatement))
	case *ExpressionStatement:
	case *ScopeStatement:
	case *IfStatement:
	case *FnStatement:
	case *StructStatement:
	case *LetStatement:
	default:
		log.Fatal("cannot generate bytecode for this statement yet")
	}

	return uint(0)
}

func (self *IRBuilder) GenerateBytecodeForReturn(statement *ReturnStatement) uint {
	returnInstruction := Instruction{
		instructionType: IReturn,
		highSlot:        0,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

	returnIndex := self.AddInstructionToBytecode(returnInstruction)
	expressionIndex := self.GenerateBytecodeForExpression(statement.expression)
	self.ByteCode[returnIndex].highSlot = expressionIndex
	return returnIndex
}

func (self *IRBuilder) GenerateBytecodeForExpression(expression Expression) uint {
	switch expression.(type) {
	case *NumberLiteralExpression:
		return self.GenerateBytecodeForNumberLiteral(expression.(*NumberLiteralExpression))
	case *BinaryExpression:
		return self.GenerateBytecodeForBinary(expression.(*BinaryExpression))
	default:
		log.Fatal("cannot generate bytecode for this expression yet")
	}

	return uint(0)
}

func (self *IRBuilder) GenerateBytecodeForNumberLiteral(expression *NumberLiteralExpression) uint {
	instruction := Instruction{
		instructionType: ILoadNumberLiteral,
		highSlot:        expression.number,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

	return self.AddInstructionToBytecode(instruction)
}

func (self *IRBuilder) GenerateBytecodeForBinary(expression *BinaryExpression) uint {

	var instructionType InstructionType

	// TODO: fix this mess, convert this to an binary op enum in the parser instead of token
	op := string(GetTokenSLice(expression.operator, self.TypedAST.TokenBuffer, self.TypedAST.Source))
	if op == "+" {
		instructionType = IBinaryAdd
	} else {
		log.Fatal("cant generate bytecode for this op yet")
	}

	instruction := Instruction{
		instructionType: instructionType,
		highSlot:        0,
		lowSlot:         0,
	}

	index := self.AddInstructionToBytecode(instruction)
	self.ByteCode[index].highSlot = self.GenerateBytecodeForExpression(expression.lhs)
	self.ByteCode[index].lowSlot = self.GenerateBytecodeForExpression(expression.rhs)
	return index
}

func (self *IRBuilder) AddInstructionToBytecode(instruction Instruction) uint {
	index := len(self.ByteCode)
	self.ByteCode = append(self.ByteCode, instruction)
	return uint(index)
}

func (self *IRBuilder) BuildFnStartInstruction(token Token, fnInfoIndex uint) Instruction {
	return Instruction{
		instructionType: IFnStart,
		highSlot:        token,
		lowSlot:         fnInfoIndex,
	}
}

func (self *IRBuilder) BuildFnEndInstruction() Instruction {
	return Instruction{
		instructionType: IFnEnd,
		highSlot:        UNUSED_SLOT_FLAG,
		lowSlot:         UNUSED_SLOT_FLAG,
	}
}

func (self *IRBuilder) BuildLoadNumberLiteralInstruction(number uint) Instruction {
	return Instruction{
		instructionType: ILoadNumberLiteral,
		highSlot:        number,
		lowSlot:         UNUSED_SLOT_FLAG,
	}
}

func (self *IRBuilder) BuildReturnInstruction(expressionInstructionIndex uint) Instruction {
	return Instruction{
		instructionType: IReturn,
		highSlot:        expressionInstructionIndex,
		lowSlot:         UNUSED_SLOT_FLAG,
	}
}

func (self *IRBuilder) PrintIR() {
	fmt.Println("::::: Byte Code ::::: ")
	for index, instruction := range self.ByteCode {
		fmt.Printf("%v | (%v) {%x, %x} \n", index, instruction.instructionType, instruction.highSlot, instruction.lowSlot)
	}
	fmt.Println("::::::::::::::::::::: ")
}
