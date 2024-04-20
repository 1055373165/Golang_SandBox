package main

import (
	"context"
	"flag"
	"fmt"
	"sync"
	"time"
)

/*
1) 队列：缓冲队列长度为 10，队列元素类型为 int
2）生产者：每 1s 往缓冲队列中放入一个类型为 int 的元素，队列满时阻塞等待
3）消费者：每 2s 从队列中读取一个元素并打印，队列为空时阻塞
4）主 goroutine 最大等待时间为 30s
5）要求优雅退出，即消费者 goroutine 退出前，要先消费完所有的 int
6）通过入参支持两种运行模式
wb（温饱模式）：生产速度快于消费速度
je（饥饿模式）：生产速度慢于消费速度
*/

var (
	wg sync.WaitGroup
	p  Producer
	c  Consumer
)

type Producer struct {
	Time     int
	Interval int
}

type Consumer struct {
	Time     int
	Interval int
}

func (p Producer) produce(queue chan<- int, ctx context.Context) {
	go func() {
		defer wg.Done()
		defer close(queue)
		for {
			select {
			case <-ctx.Done():
				return
			default:
				p.Time = p.Time + 1
				select {
				case queue <- p.Time:
					fmt.Printf("生产者进行第%d次生产，值：%d\n", p.Time, p.Time)
					time.Sleep(time.Duration(p.Interval) * time.Millisecond)
				case <-ctx.Done():
					return
				}
			}
		}
	}()
}

func (c Consumer) consume(queue <-chan int, ctx context.Context) {
	go func() {
		defer wg.Done()
		for {
			select {
			case val, ok := <-queue:
				if !ok {
					// 如果通道被关闭，则返回
					return
				}
				c.Time++
				fmt.Printf("-->消费者进行第%d次消费，值：%d\n", c.Time, val)
				time.Sleep(time.Duration(c.Interval) * time.Millisecond)
			case <-ctx.Done():
				for val := range queue {
					fmt.Printf("---> 消费者：优雅消费，值为：%v\n", val)
				}
				return
			}
		}
	}()
}

func main() {
	wg.Add(2)

	timeout := 5
	ctx, cancel := context.WithTimeout(context.Background(), time.Duration(timeout)*time.Second)
	defer cancel()

	queue := make(chan int, 10)
	c.consume(queue, ctx)
	p.produce(queue, ctx)

	fmt.Println("main waitting...")
	wg.Wait()
	fmt.Println("done")
}

func init() {
	mode := flag.String("m", "wb", "请输入运行模式：\nwb（温饱模式）生产速度快于消费速度\nje（饥饿模式）生产速度慢于消费速度")
	flag.Parse()

	p = Producer{}
	c = Consumer{}

	if *mode == "wb" {
		fmt.Println("运行模式：wb（温饱模式）生产速度快于消费速度")
		p.Interval = 100 // 每隔 100ms 生产一次
		c.Interval = 200 // 每隔 200ms 消费一次
	} else {
		fmt.Println("运行模式：je（饥饿模式）生产速度慢于消费速度")
		p.Interval = 200 // 每隔 200ms 生产一次
		c.Interval = 100 // 每个 100ms 消费一次
	}
}
