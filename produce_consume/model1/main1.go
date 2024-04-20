package main

import (
	"fmt"
	"time"
)

/*
1. producer函数是生产者函数，它通过通道将数据发送到消费者。

2. consumer函数是消费者函数，它从通道中接收数据并进行消费。

3. main函数是程序的入口，它创建了一个整型通道和一个用于通知消费者完成的通道。

4. 通过go关键字，在main函数中启动了生产者和消费者的goroutine。

5. 生产者不断地向通道发送数据，而消费者通过range语句从通道中循环接收数据，并进行相应的处理。

6. 当通道被关闭后，消费者goroutine会退出循环，并向done通道发送一个通知，表示消费者已完成。

7. 最后，主线程通过<-done语句等待消费者完成，一旦收到通知，输出相应的消息，程序执行完毕。
*/

func producer(ch chan<- int) {
	for i := 1; i <= 5; i++ {
		ch <- i
		fmt.Println("生产者生产了数据：", i)
		time.Sleep(time.Millisecond * 500) // 模拟生产过程
	}
	// 生产完毕后关闭数据通道
	close(ch)
}

func consumer(ch <-chan int, done chan<- bool) {
	for num := range ch {
		fmt.Println("消费者消费：", num)
		time.Sleep(time.Second) // 模拟消费过程
	}
	// 消费完成后给主 goroutine 发送信号
	done <- true
}

func main() {
	ch := make(chan int, 3) // 有缓冲的通道作为任务队列
	done := make(chan bool) // 用于主 goroutine 和其他 goroutine 的协作

	go producer(ch)
	go consumer(ch, done)

	// main goroutine blocking for waiting other goroutine over
	<-done
	fmt.Println("done...")
}
