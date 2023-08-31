package compiler

import (
	"fmt"
)

type InstructionType = uint8

const UNUSED_SLOT_FLAG uint = 0x_fcfc_fcfc_fcfc_fcfc

const (
	ILoadNumberLiteral InstructionType = iota

	// :::::::::::::::::::::::::::::::::::
	// above expressions, below statements
	// :::::::::::::::::::::::::::::::::::

	IFnStart
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

	return nil
}

func (self *IRBuilder) BuildFnStartInstruction(token Token, fnInfoIndex uint) Instruction {
	return Instruction{
		instructionType: IFnStart,
		highSlot:        token,
		lowSlot:         fnInfoIndex,
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
	fmt.Println("::: FN Info table ::: ")
	fmt.Println("::::::::::::::::::::: ")
}
