#include "udp_socket.hpp"
#include <cassert>
#include <system_error>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

udp_socket::udp_socket(unsigned short port)
  : socket_(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
{
  if (socket_ == -1)
  {
    std::error_code ec(errno, std::generic_category());
    throw std::system_error(ec, "udp_socket creation");
  }

  union
  {
    sockaddr base;
    sockaddr_in v4;
  } addr;

  addr.v4.sin_family = AF_INET;
  addr.v4.sin_addr.s_addr = 0;
  addr.v4.sin_port = htons(port);

  if (::bind(socket_, &addr.base, sizeof(addr.v4)) == -1)
  {
    std::error_code ec(errno, std::generic_category());
    ::close(socket_);
    throw std::system_error(ec, "udp_socket bind");
  }
}

udp_socket::~udp_socket()
{
  ::close(socket_);
}

void udp_socket::connect(const std::string& ip, unsigned short port)
{
  union
  {
    sockaddr base;
    sockaddr_in v4;
  } addr;

  addr.v4.sin_family = AF_INET;
  addr.v4.sin_port = htons(port);

  switch (::inet_pton(AF_INET, ip.c_str(), &addr.v4.sin_addr))
  {
  default:
    assert(0);
    break;
  case 1:
    break;
  case 0:
    throw std::system_error(make_error_code(std::errc::invalid_argument), "udp_socket connect");
  case -1:
    std::error_code ec(errno, std::generic_category());
    throw std::system_error(ec, "udp_socket connect");
  }

  if (::connect(socket_, &addr.base, sizeof(addr.v4)) == -1)
  {
    std::error_code ec(errno, std::generic_category());
    throw std::system_error(ec, "udp_socket bind");
  }
}

void udp_socket::send(const void* data, std::size_t length)
{
  for (;;)
  {
    ssize_t n = ::send(socket_, data, length, 0);
    if (n == -1 && errno != EINTR)
      break;
  }
}

std::size_t udp_socket::receive(void* data, std::size_t length)
{
  for (;;)
  {
    ssize_t n = ::recv(socket_, data, length, 0);
    if (n > 0)
      return n;
  }
}
