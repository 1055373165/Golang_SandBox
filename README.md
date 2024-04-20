# Learning Go

## Introduction
This is just a sandbox repository for me to put in random programming, mostly for me to learn go. The motivation is that I want to learn a new backend language to be able to write high-performance backend services, and there are many projects (k8s, docker, normad) are written in go. If i understand Go it will be easier for me to understand these projects.

What i like about Go:

- Clear and nice syntax.
- Channel and goroutine is awesome, a lot more cleaner than in python where i have to use Condition for cordination.
- Concurrency is easy to reason. Everything is blocking, unless you let it go.
- Strong-typed
- Fast compilation
- What I don't like about Go:

Single workspace is a confusing.
- Error handling is just awful. It's too bad that Go does not have a try / catch block.
- JSON deserialization is hard if you deal with generic interface
- You tend to write more code due to the verbose syntax of Go
- Lacking of Generic (coming soon)

## I/O
- Use bytes.Buffer for string writer
- Use strings.Reader for string reader

## Channels
It’s okie to leave channel open. GC will collect it
- http://www.tapirgames.com/blog/golang-channel-closing

## Ser/der
- Gobs of data
- Handling interace{} decoding with Gob
- Go codec series

## Slices
- Go slice is passed as reference.
- Use copy() to copy slice
- Use []byte(str) to convert string to byte array.
- Use string(bytes) to convert byte to string.
- Remove an item in slice
- Use reflect.DeepEqual to compare two slices

## Links
- https://rakyll.org/style-packages
- https://blog.golang.org/package-names
- appliedgo.net
[Go by examples: Errors](https://gobyexample.com/errors)
[Traps, Gotchas, and Common Mistakes for New Golang Devs](http://devs.cloudimmunity.com/gotchas-and-common-mistakes-in-go-golang/)
