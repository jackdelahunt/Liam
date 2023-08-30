package main

import (
	"fmt"
	"github.com/jackdelahunt/Liam/gliamc/src/liamCLI"
)

const (
	Code = `
fn main() void {
	if true {
		return 5;
	}
}
`
)

func main() {
	err := liamCLI.StartCLI()
	if err != nil {
		fmt.Println(err.Error())
	}
}
