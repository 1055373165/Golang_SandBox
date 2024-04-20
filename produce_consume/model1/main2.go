package main

import (
	"fmt"
	"sync"
	"time"
)

type Data struct {
	Value int
}

type Queue struct {
	mutex      sync.Mutex
	cond       *sync.Cond
	buffer     []Data
	terminated bool
}

func NewQueue() *Queue {
	q := &Queue{}
	q.cond = sync.NewCond(&q.mutex) // cond 必须和 mutex 绑定才能使用
	return q
}

func (q *Queue) Produce(data Data) {
	q.mutex.Lock()
	defer q.mutex.Unlock()

	q.buffer = append(q.buffer, data)
	fmt.Printf("Produced: %d\n", data.Value)

	q.cond.Signal() // 唤醒等待中的消费者
}

func (q *Queue) Consume() Data {
	q.mutex.Lock()
	defer q.mutex.Unlock()

	for len(q.buffer) == 0 && !q.terminated { // wait for data available
		q.cond.Wait()
	}
	// return from blocking and receive data
	if len(q.buffer) > 0 {
		data := q.buffer[0]
		q.buffer = q.buffer[1:]
		fmt.Printf("Consumed: %d\n", data.Value)
		return data
	}

	return Data{}
}

func (q *Queue) Terminated() {
	q.mutex.Lock()
	defer q.mutex.Unlock()
	// 使用互斥锁保护 Queue 的各个字段的并发安全
	q.terminated = true
	// wake up all consumer
	q.cond.Broadcast()
}

func main() {
	queue := NewQueue()

	// start producer
	for i := 1; i <= 3; i++ {
		// producer:goroutine = 1:1
		go func(id int) {
			for j := 1; j <= 5; j++ {
				data := Data{Value: id*10 + j}
				queue.Produce(data)
				time.Sleep(time.Millisecond * 500) // 模拟生产时间
			}
		}(i)
	}
	// start consumer
	for i := 1; i <= 2; i++ {
		go func(id int) {
			for {
				data := queue.Consume()
				if data.Value == 0 {
					break
				}
				time.Sleep(1 * time.Second) // 模拟消费数据时间
			}
		}(i)
	}

	// 等待一定时间后终止消费者
	time.Sleep(6 * time.Second)
	queue.Terminated()

	// 等待生产者和消费者完成
	time.Sleep(time.Second * 1)
}
