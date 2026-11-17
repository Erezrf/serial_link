CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2 -pthread
PROTO   = protocol/crc16.c protocol/frame.c protocol/parser.c

all: test_protocol test_robust test_robust_xor test_ring_buffer demo_uart

test_protocol: tests/test_protocol.c $(PROTO)
	$(CC) $(CFLAGS) -o $@ $^

test_robust: apps/test_robust.c hal/channel.c $(PROTO)
	$(CC) $(CFLAGS) -o $@ $^

# same program, but the frame check is an 8-bit XOR instead of CRC-16
test_robust_xor: apps/test_robust.c hal/channel.c $(PROTO)
	$(CC) $(CFLAGS) -DCHECK_XOR -o $@ $^

test_ring_buffer: tests/test_ring_buffer.c drivers/ring_buffer.c
	$(CC) $(CFLAGS) -o $@ $^

demo_uart: apps/demo_uart.c hal/uart_sim.c hal/channel.c drivers/ring_buffer.c $(PROTO)
	$(CC) $(CFLAGS) -o $@ $^

# build everything and run every test and demo in order
run: all
	./test_protocol
	./test_ring_buffer
	./test_robust
	-./test_robust_xor          # the leading '-' : this one is EXPECTED to fail (exit 1)
	./demo_uart

clean:
	rm -f test_protocol test_robust test_robust_xor test_ring_buffer demo_uart 

.PHONY: all run clean