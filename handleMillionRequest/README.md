# Run

`go run main.go -max_workers 5`

# Test

`for i in {1..15}; do curl localhost:8080/work -d name=job$i -d delay=$(expr $i % 9 + 1)s; done`