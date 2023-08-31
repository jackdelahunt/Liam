package compiler

import (
	"fmt"
	"log"
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

var (
	BuiltInTypes = [...]BaseTypeInfo{
		{
			Name:      "bool",
			IsBuiltIn: true,
		},
	}
)

type TypeInfo interface {
}

type BaseTypeInfo struct {
	Name      string
	IsBuiltIn bool
}

type FnInfo struct {
	Name       string
	ReturnType TypeInfo
}

type Instruction struct {
	instructionType InstructionType
	highSlot        uint
	lowSlot         uint
}

type IRBuilder struct {
	AST                AST
	FnInfoBuffer       []FnInfo
	BaseTypeInfoBuffer []BaseTypeInfo
	ByteCode           []Instruction
}

func NewIRBuilder(ast AST) *IRBuilder {
	return &IRBuilder{
		AST:                ast,
		FnInfoBuffer:       make([]FnInfo, 0),
		BaseTypeInfoBuffer: make([]BaseTypeInfo, 0),
		ByteCode:           make([]Instruction, 0),
	}
}

func (self *IRBuilder) BuildIR() error {
	self.FillInBaseTypeInfo()

	err := self.FillInFnInfo()
	if err != nil {
		return err
	}

	return nil
}

func (self *IRBuilder) FillInBaseTypeInfo() {
	// adding the builtin types
	for _, info := range BuiltInTypes {
		self.BaseTypeInfoBuffer = append(self.BaseTypeInfoBuffer, info)
	}

	for _, structStatement := range self.AST.StructStatements {
		self.BaseTypeInfoBuffer = append(self.BaseTypeInfoBuffer, BaseTypeInfo{
			Name:      string(GetTokenSLice(structStatement.identifier, self.AST.TokenBuffer, self.AST.Source)),
			IsBuiltIn: false,
		})
	}
}

func (self *IRBuilder) FillInFnInfo() error {
	for _, fnStatement := range self.AST.FnStatements {
		returnType, err := self.GetTypeInfoFromTypeExpression(fnStatement.returnType)
		if err != nil {
			return err
		}

		self.FnInfoBuffer = append(self.FnInfoBuffer, FnInfo{
			Name:       string(GetTokenSLice(fnStatement.identifier, self.AST.TokenBuffer, self.AST.Source)),
			ReturnType: returnType,
		})
	}

	return nil
}

// func (self *IRBuilder) GenerateByteCode() error {

// }

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

	for index, info := range self.FnInfoBuffer {
		fmt.Printf("%v :: %v\n", index, info.Name)
	}

	fmt.Println("::::::::::::::::::::: ")
}

func (self *IRBuilder) GetTypeInfoFromTypeExpression(typeExpression TypeExpression) (TypeInfo, error) {
	switch typeExpression.(type) {
	case *IdentifierTypeExpression:
		identifierTypeExpression := typeExpression.(*IdentifierTypeExpression)
		identifierString := string(GetTokenSLice(identifierTypeExpression.identifier, self.AST.TokenBuffer, self.AST.Source))
		for _, baseType := range self.BaseTypeInfoBuffer {
			if baseType.Name == identifierString {
				return baseType, nil
			}
		}

		return nil, fmt.Errorf(fmt.Sprintf("type '%v' does not exist", identifierString))
	default:
		log.Fatal("unhandled type expression type in ir builder")
		return nil, nil // this is a redundant return
	}
}
