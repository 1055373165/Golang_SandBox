package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
	"time"
)

// Job
type Job struct {
	name string        // 任务名称
	used time.Duration // 任务执行耗时
}

// Worker
type Worker struct {
	id         int        // 工作者唯一标识
	workerPool WorkerPool // 工作队列池
	jobChan    chan Job   // 用于接收 Job 的工作队列（Chan Local Storage）
	quit       chan bool  // 接收退出信号
}

func NewWorker(workerId int, workerPool WorkerPool) *Worker {
	return &Worker{
		id:         workerId,
		workerPool: workerPool,
		quit:       make(chan bool),
		jobChan:    make(chan Job),
	}
}

func (w Worker) start() {
	go func() {
		for {
			w.workerPool <- w.jobChan
			select {
			case job := <-w.jobChan:
				fmt.Printf("worker%d: started %s, blocking for %f seconds\n", w.id, job.name, job.used.Seconds())
				time.Sleep(job.used)
				fmt.Printf("worker%d: completed %s\n", w.id, job.name)
			case <-w.quit:
				fmt.Printf("worker%s stopping...\n", w.id)
				return
			}
		}
	}()
}

func (w Worker) stop() {
	go func() {
		w.quit <- true
	}()
}

// WorkerPool
type WorkerPool chan chan Job

// dispatcher
type dispatcher struct {
	maxWorkers int        // 最大工作者数量（最大并发工作线程）
	jobQueue   chan Job   // 全局 Job 分发队列
	workerPool WorkerPool // 管理工作队列池
}

func NewDispatcher(maxWorkers int, jobQueue chan Job) *dispatcher {
	workerPool := make(chan chan Job, maxWorkers)
	return &dispatcher{
		maxWorkers: maxWorkers,
		jobQueue:   jobQueue,
		workerPool: workerPool,
	}
}

func (d *dispatcher) run() {
	for i := 0; i < d.maxWorkers; i++ {
		worker := NewWorker(i+1, d.workerPool)
		worker.start()
	}

	go d.dispatch()
}

func (d *dispatcher) dispatch() {
	for job := range d.jobQueue {
		go func() {
			fmt.Printf("fetching available worker job chan for: %s\n", job.name)
			availableWorkerJobChan := <-d.workerPool
			fmt.Printf("adding %s to worker job chan\n", job.name)
			availableWorkerJobChan <- job
		}()
	}
}
func requestHandler(w http.ResponseWriter, r *http.Request, jobQueue chan Job) {
	if r.Method != "POST" {
		w.Header().Set("Allow", "POST")
		w.WriteHeader(http.StatusMethodNotAllowed)
		return
	}

	delay, err := time.ParseDuration(r.FormValue("delay"))
	if err != nil {
		http.Error(w, "Bad delay value: "+err.Error(), http.StatusBadRequest)
		return
	}

	if delay.Seconds() < 1 || delay.Seconds() > 10 {
		http.Error(w, "The delay must be between 1 and 10 seconds, inclusively", http.StatusBadRequest)
		return
	}

	name := r.FormValue("name")
	if name == "" {
		http.Error(w, "You must specify a name", http.StatusBadRequest)
	}

	job := Job{name: name, used: delay}
	jobQueue <- job
	w.WriteHeader(http.StatusCreated)
}

func main() {
	var (
		maxWorkers   = flag.Int("max_workers", 5, "the number of concurrent worker")
		maxQueueSize = flag.Int("max_queue_size", 100, "the size of job queue")
		port         = flag.String("port", "8080", "the server port")
	)

	flag.Parse()

	// create a job queue
	jobQueue := make(chan Job, *maxQueueSize)
	// start a dispatcher
	dispatcher := NewDispatcher(*maxWorkers, jobQueue)
	dispatcher.run()

	// register http route (net/http Underlying concurrency is handled.)
	http.HandleFunc("/work", func(w http.ResponseWriter, r *http.Request) {
		requestHandler(w, r, jobQueue)
	})

	log.Fatal(http.ListenAndServe(":"+*port, nil))
}
