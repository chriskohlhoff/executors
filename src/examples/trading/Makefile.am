AUTOMAKE_OPTIONS = subdir-objects

noinst_PROGRAMS = \
	trading_server \
	enter_one_order \
	market_data_dump

AM_CXXFLAGS = -I$(srcdir)/../../../include

trading_server_SOURCES = \
	common/order_management.cpp \
	common/order_side.cpp \
	common/market_data.cpp \
	common/udp_socket.cpp \
	server/connection_handler.cpp \
	server/market_by_order.cpp \
	server/market_data_bus.cpp \
	server/order_book.cpp \
	server/order_management_bus.cpp \
	server/price_time_order_book.cpp \
	server/server.cpp

enter_one_order_SOURCES = \
	common/order_management.cpp \
	common/order_side.cpp \
	common/udp_socket.cpp \
	client/enter_one_order.cpp

market_data_dump_SOURCES = \
	common/market_data.cpp \
	common/order_side.cpp \
	common/udp_socket.cpp \
	client/market_data_dump.cpp

EXTRA_DIST = \
	common/order_side.hpp \
	common/market_data.hpp \
	common/udp_socket.hpp \
	common/order_management.hpp \
	server/order_management_bus.hpp \
	server/price_time_order_book.hpp \
	server/connection_handler.hpp \
	server/order_book.hpp \
	server/market_data_bus.hpp \
	server/market_by_order.hpp \
	server/market_data_feed.hpp

MAINTAINERCLEANFILES = \
	$(srcdir)/Makefile.in
