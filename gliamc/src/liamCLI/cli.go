package liamCLI

import (
	"fmt"
	cli "github.com/urfave/cli/v2"
	"os"
)

func StartCLI() error {
	app := newApp()
	err := app.Run(os.Args)
	if err != nil {
		return err
	}

	return nil
}

func newApp() *cli.App {
	return &cli.App{
		Name:  "gliamc",
		Usage: "the liam compiler implemented in go",
		Commands: []*cli.Command{
			newCompileCommand(),
		},
	}
}

func newCompileCommand() *cli.Command {

	var path string
	compileAction := func(context *cli.Context) error {
		if path == "" {
			fmt.Println("its nil")
		}
		return nil
	}

	return &cli.Command{
		Name:    "compile",
		Aliases: nil,
		Usage:   "compile a program",
		Action:  compileAction,
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:        "path",
				Usage:       "path to input file",
				Destination: &path,
			},
		},
	}
}
