package main

import (
	"fmt"
	"sync"
	"time"
)

const (
	PRODUCERS      = 5
	CONSUMERS      = 2
	PRODUCTS       = 20
	TOTAL_PRODUCTS = PRODUCERS * PRODUCTS // Total products to be produced
)

var (
	productCount  int // 已生产还未消费的产品
	producedCount int // 总共生产的产品数量
	lock          sync.Mutex
	wg            sync.WaitGroup
	condition     *sync.Cond
	done          bool // 退出标志
)

func Produce(index int) {
	defer wg.Done()

	for {
		lock.Lock()
		for productCount >= PRODUCTS && !done {
			condition.Wait()
		}

		if done {
			lock.Unlock()
			return
		}

		productCount++
		producedCount++
		fmt.Printf("Producer %d produced, products count is %d\n", index, productCount)

		if producedCount >= TOTAL_PRODUCTS {
			done = true
			condition.Broadcast() // Notify all goroutines to exit
		} else {
			condition.Signal() // Notify one goroutine to proceed
		}

		lock.Unlock()
		time.Sleep(time.Millisecond * 100)
	}
}

func Consume(index int) {
	defer wg.Done()

	for {
		lock.Lock()
		for productCount <= 0 && !done {
			condition.Wait()
		}

		if done {
			for i := productCount; i >= 1; i-- {
				fmt.Printf("last Consumer %d consumed, last products count is %d\n", index, i)
			}
			lock.Unlock()
			return
		}

		productCount--
		fmt.Printf("Consumer %d consumed, products count is %d\n", index, productCount)
		condition.Signal()
		lock.Unlock()

		time.Sleep(time.Millisecond * 100)
	}
}

func main() {
	condition = sync.NewCond(&lock)

	wg.Add(PRODUCERS + CONSUMERS)
	for i := 0; i < CONSUMERS; i++ {
		go Consume(i)
	}
	for i := 0; i < PRODUCERS; i++ {
		go Produce(i)
	}
	wg.Wait()
	fmt.Println("All products have been produced and consumed, program exiting.")
}
