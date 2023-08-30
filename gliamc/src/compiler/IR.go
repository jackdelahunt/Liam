package compiler

type TypeInfo interface {
}

type FnTypeInfo struct {
	Name       string
	ReturnType TypeInfo
}

type IRBuilder struct {
	AST     AST
	FnTypes []FnTypeInfo
}

func NewIRBuilder(ast AST) *IRBuilder {
	return &IRBuilder{
		AST:     ast,
		FnTypes: make([]FnTypeInfo, 0),
	}
}

func (self *IRBuilder) BuildIR() {
	self.CreateFnTypes()
}

func (self *IRBuilder) CreateFnTypes() {
	for _, fnStatement := range self.AST.FnStatements {
		self.FnTypes = append(self.FnTypes, FnTypeInfo{
			Name:       string(GetTokenSLice(fnStatement.identifier, self.AST.TokenBuffer, self.AST.Source)),
			ReturnType: nil,
		})
	}
}
