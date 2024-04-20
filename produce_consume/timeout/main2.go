package main

import (
	"fmt"
	"time"
)

/*
	1) 多通道
	2）定时器：设置主 goroutine 最大等待时间
*/

func main() {
	ch1 := make(chan int, 10)
	go func(ch chan<- int) { // 参数为只能接收的通道
		// 假设子 goroutine 是一个耗时操作，需要 10s 后才能从网络中获取数据
		time.Sleep(10 * time.Second)
		ch <- 1
	}(ch1)

	timer := time.NewTimer(5 * time.Second) // 设置定时器的超时时间，主 goroutine 只等 5s
	fmt.Println("select start...")
	select {
	case <-ch1:
		fmt.Println("从 channel1 收到一个数字")
	case <-timer.C:
		fmt.Println("5s最大超时到了, main goroutine 退出")
	}

	fmt.Println("done")
}
