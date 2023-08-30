package main

import "fmt"

const (
	Code = `
fn main() void {
	return 5;
	
	if true {
		return x;
	}
}
`
)

func main() {

	source := []rune("012sailor012")
	// token - start: 3, end: 8
	slice := source[3:9]

	fmt.Printf("%v\n", string(slice))

	//err := liamCLI.StartCLI()
	//if err != nil {
	//	fmt.Println(err.Error())
	//}
}
