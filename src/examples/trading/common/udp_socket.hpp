//
// udp_socket.hpp
// ~~~~~~~~~~~~~~
// Simple UDP socket wrapper.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <cstddef>
#include <string>

// A very simple wrapper for UDP sockets.
class udp_socket
{
public:
  udp_socket(const udp_socket&) = delete;
  udp_socket& operator=(const udp_socket&) = delete;

  // Open and bind the socket.
  explicit udp_socket(unsigned short port);

  // Closes the socket.
  ~udp_socket();

  // Connect the socket to the specified remote IP address and port.
  void connect(const std::string& ip, unsigned short port);

  // Send a datagram on the socket.
  void send(const void* data, std::size_t length);

  // Synchronously receive a datagram on the socket.
  std::size_t receive(void* data, std::size_t length);

private:
  // The UDP socket's file descriptor.
  int socket_;
};

#endif
