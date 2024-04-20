package main

import (
	"fmt"
	"time"
)

func main() {
	messages := make(chan int, 10)
	done := make(chan bool)

	defer close(messages)

	// start a consumer goroutine
	go func() {
		ticker := time.NewTicker(1 * time.Second)
		for range ticker.C {
			select {
			case <-done:
				fmt.Println("child process interrupt..") // 数据还没收完就被停止了
				return
			default:
				fmt.Printf("receive message: %d\n", <-messages)
			}
		}
	}()

	// main groutine as producer
	for i := 0; i < 10; i++ {
		messages <- i
	}

	// blocking for wait
	time.Sleep(time.Second * 5)
	close(done) // <-done 触发
	time.Sleep(time.Second)
	fmt.Println("main process exit!")
}
