package protocol

import (
	"fmt"
	"io"
)

type CommandWiter struct {
	writer io.Writer
}

func NewCommandWriter(writer io.Writer) *CommandWiter {
	return &CommandWiter{writer: writer}
}

func (w *CommandWiter) writeString(msg string) error {
	_, err := w.writer.Write([]byte(msg))
	return err
}

func (w *CommandWiter) Write(command interface{}) error {
	var err error

	switch v := command.(type) {
	case SendCommand:
		err = w.writeString(fmt.Sprintf("SEND %v\n", v.Message))
	case MessageCommand:
		err = w.writeString(fmt.Sprintf("MESSAGE %v %v\n", v.Name, v.Message))
	case NameCommand:
		err = w.writeString(fmt.Sprintf("NAME %v\n", v.Name))
	default:
		err = UnknownCommand
	}
	return err
}
