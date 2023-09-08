package compiler

import (
	"fmt"
	"log"
)

type InstructionType = uint8

const UNUSED_SLOT_FLAG uint = 0x_cccc_cccc

const (
	ILoadNumberLiteral InstructionType = iota
	IBinaryAdd
	ICall

	// :::::::::::::::::::::::::::::::::::
	// above expressions, below statements
	// :::::::::::::::::::::::::::::::::::

	IFnStart
	IFnEnd
	IReturn
	IIfStart
	IIfEnd
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
	start := Instruction{
		instructionType: IFnStart,
		highSlot:        index,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

	end := Instruction{
		instructionType: IFnEnd,
		highSlot:        UNUSED_SLOT_FLAG,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

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
	case *IfStatement:
		return self.GenerateBytecodeForIf(statement.(*IfStatement))
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

func (self *IRBuilder) GenerateBytecodeForIf(statement *IfStatement) uint {
	ifStart := Instruction{
		instructionType: IIfStart,
		highSlot:        0,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

	ifEnd := Instruction{
		instructionType: IIfEnd,
		highSlot:        UNUSED_SLOT_FLAG,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

	ifStartIndex := self.AddInstructionToBytecode(ifStart)

	expressionIndex := self.GenerateBytecodeForExpression(statement.condition)
	self.ByteCode[ifStartIndex].highSlot = expressionIndex

	for _, subStatement := range statement.scope.statements {
		self.GenerateBytecodeForStatement(subStatement)
	}

	ifEndIndex := self.AddInstructionToBytecode(ifEnd)

	// low slot of start is the end index
	// high slot of the end is the start index
	self.ByteCode[ifStartIndex].lowSlot = ifEndIndex
	self.ByteCode[ifEndIndex].highSlot = ifStartIndex

	return ifStartIndex
}

func (self *IRBuilder) GenerateBytecodeForExpression(expression Expression) uint {
	switch expression.(type) {
	case *NumberLiteralExpression:
		return self.GenerateBytecodeForNumberLiteral(expression.(*NumberLiteralExpression))
	case *BinaryExpression:
		return self.GenerateBytecodeForBinary(expression.(*BinaryExpression))
	case *CallExpression:
		return self.GenerateBytecodeForCall(expression.(*CallExpression))
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

	tokenType := self.TypedAST.TokenBuffer[expression.operator].TokenType
	switch tokenType {
	case Plus:
		instructionType = IBinaryAdd
	case Minus:
	case Star:
	case Divide:
	default:
		log.Fatal("unsupported binary op in IR builder")
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

func (self *IRBuilder) GenerateBytecodeForCall(expression *CallExpression) uint {
	fnTypeInfo, ok := expression.callee.TypeInfo().(*FnTypeInfo)
	if !ok {
		log.Fatal("this should always be a fn")
		return 0
	}

	instruction := Instruction{
		instructionType: ICall,
		highSlot:        fnTypeInfo.FnTypeIndex,
		lowSlot:         UNUSED_SLOT_FLAG,
	}

	return self.AddInstructionToBytecode(instruction)
}

func (self *IRBuilder) AddInstructionToBytecode(instruction Instruction) uint {
	index := len(self.ByteCode)
	self.ByteCode = append(self.ByteCode, instruction)
	return uint(index)
}

func GetInstructionTypeName(instructionType InstructionType) string {
	switch instructionType {
	case ILoadNumberLiteral:
		return "number_lit"
	case IBinaryAdd:
		return "binary_add"
	case ICall:
		return "call"
	case IFnStart:
		return "fn_start"
	case IFnEnd:
		return "fn_end"
	case IReturn:
		return "return"
	case IIfStart:
		return "if_start"
	case IIfEnd:
		return "if_end"

	default:
		log.Fatal(fmt.Sprintf("cannot get name of instruction type %v", instructionType))
	}

	return ""
}

func (self *IRBuilder) PrintIR() {

	fmt.Println(":::::::::::::::::: Fn Table ::::::::::::::::::")
	fmt.Println("index | name")
	fmt.Println("----------------------------------------------")
	for index, fn := range self.TypedAST.FnTypeInfos {
		fmt.Printf("%-5v | %v \n", index, string(GetTokenSLice(fn.Identifier, self.TypedAST.TokenBuffer, self.TypedAST.Source)))
	}
	fmt.Println("::::::::::::::::::::::::::::::::::::::::::::::")

	fmt.Print("\n\n")

	fmt.Println(":::::::::::::::::: Byte Code :::::::::::::::::")
	fmt.Println("index | type       | [high       , low       ]")
	fmt.Println("----------------------------------------------")
	for index, instruction := range self.ByteCode {
		fmt.Printf("%-5v | %-10v | [%-10x , %-10x] \n", index, GetInstructionTypeName(instruction.instructionType), instruction.highSlot, instruction.lowSlot)
	}
	fmt.Println("::::::::::::::::::::::::::::::::::::::::::::::")
}
