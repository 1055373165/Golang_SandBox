package server

import (
	"chatserver/protocol"
	"errors"
	"io"
	"log"
	"net"
	"sync"
)

type client struct {
	conn   net.Conn
	name   string
	writer *protocol.CommandWiter
}

type TcpChatServer struct {
	listener net.Listener
	clients  []*client
	mutex    *sync.Mutex
}

var (
	UnknownClient = errors.New("Unknown client")
)

func NewServer() *TcpChatServer {
	return &TcpChatServer{
		mutex: &sync.Mutex{},
	}
}

func (s *TcpChatServer) Listen(address string) error {
	l, err := net.Listen("tcp", address)
	if err == nil {
		s.listener = l
	}
	log.Printf("Listening on %v", address)
	return err
}

func (s *TcpChatServer) Start() {
	for {
		conn, err := s.listener.Accept()
		if err != nil {
			log.Print(err)
		} else {
			client := s.accept(conn)
			go s.serve(client)
		}
	}
}

func (s *TcpChatServer) Broadcast(command interface{}) error {
	for _, client := range s.clients {
		client.writer.Write(command)
	}
	return nil
}

func (s *TcpChatServer) Send(name string, command interface{}) error {
	for _, client := range s.clients {
		if client.name == name {
			return client.writer.Write(command)
		}
	}
	return UnknownClient
}

func (s *TcpChatServer) Close() {
	s.listener.Close()
}

func (s *TcpChatServer) accept(conn net.Conn) *client {
	log.Printf("Accepting connection from %v, total clients: %v", conn.RemoteAddr().String(), len(s.clients)+1)
	s.mutex.Lock()
	defer s.mutex.Unlock()

	client := &client{
		conn:   conn,
		writer: protocol.NewCommandWriter(conn),
	}
	s.clients = append(s.clients, client)
	return client
}

func (s *TcpChatServer) remove(client *client) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	for i, check := range s.clients {
		if check == client {
			s.clients = append(s.clients[:i], s.clients[i+1:]...)
		}
	}
	log.Printf("closing connection from %v", client.conn.RemoteAddr().String())
	client.conn.Close()
}

func (s *TcpChatServer) serve(client *client) {
	// 从 client 代表的 tcp 连接中读取数据
	cmdReader := protocol.NewCommandReader(client.conn)

	defer s.remove(client)

	for {
		cmd, err := cmdReader.Read()
		if err != nil && err != io.EOF {
			log.Printf("Read error: %v", err)
		}

		if cmd != nil {
			switch v := cmd.(type) {
			case protocol.SendCommand:
				go s.Broadcast(protocol.MessageCommand{
					Message: v.Message,
					Name:    client.name,
				})
			case protocol.NameCommand:
				client.name = v.Name
			}
		}

		if err == io.EOF {
			break
		}
	}
}
