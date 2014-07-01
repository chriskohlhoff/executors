#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <cstddef>
#include <experimental/executor>
#include <experimental/thread_pool>
#include <string>

// A very simple wrapper for UDP sockets.
class udp_socket
{
public:
  // Open and bind the socket.
  explicit udp_socket(unsigned short port);

  // Closes the socket.
  ~udp_socket();

  // Connect the socket to the specified remote IP address and port.
  void connect(const std::string& ip, unsigned short port);

  // Send a datagram on the socket.
  void send(const void* data, std::size_t length);

  // Asynchronously receive a datagram on the socket.
  template <class CompletionToken>
    auto async_receive(void* data, std::size_t length, CompletionToken&& token);

private:
  // Synchronously receive a datagram on the socket.
  std::size_t receive(void* data, std::size_t length);

  // The thread pool used for simulating asynchronous receives.
  std::experimental::thread_pool thread_pool_{1};

  // The UDP socket's file descriptor.
  int socket_;
};

template <class CompletionToken>
auto udp_socket::async_receive(void* data,
    std::size_t length, CompletionToken&& token)
{
  return std::experimental::defer(
      [=]{ return receive(data, length); },
      std::forward<CompletionToken>(token));
}

#endif
