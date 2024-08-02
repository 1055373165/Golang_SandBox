package main

import (
	"fmt"
	"sync"
)

type Foo struct {
	wg *sync.WaitGroup
	c1 chan bool
	c2 chan bool
}

func (foo *Foo) first() {
	defer foo.wg.Done()
	fmt.Print("first")
	// first 完成后往 c1 通道中发送一个信号
	foo.c1 <- true
}

func (foo *Foo) second() {
	defer foo.wg.Done()
	//  second 必须等待 first 完成
	<-foo.c1

	fmt.Print("second")
	// second 完成后往 c2 通道中发送一个信号
	foo.c2 <- true
}

func (foo *Foo) third() {
	defer foo.wg.Done()
	// third 必须等待 second 完成
	<-foo.c2

	fmt.Print("thrid")
}

func run(input []int) {
	wg := &sync.WaitGroup{}
	c1 := make(chan bool)
	c2 := make(chan bool)
	foo := Foo{wg, c1, c2}

	m := map[int]func(){
		1: foo.first,
		2: foo.second,
		3: foo.third,
	}

	/*
		m = make(map[int]func())
		m[1] = foo.first
		m[2] = foo.second
		m[3] = foo.third
	*/

	wg.Add(3)
	for _, v := range input {
		go m[v]()
	}
	wg.Wait()
}

func main() {
	run([]int{3, 2, 1})
}
