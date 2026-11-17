# serial_link

A simulation and test suite demonstrating reliable binary frame parsing and interrupt-driven UART communication over an unreliable serial link.

## Part 1: Floating-Point Frame Parsing & Link Robustness (`test_robust.c`)

* **What was implemented**:
  * A binary framing protocol (`frame.c`) that encodes floating-point payloads with sync headers, message IDs, and integrity checksums.
  * A byte-by-byte state-machine parser (`parser.c`) designed to resynchronize automatically after dropped or corrupted bytes.
  * A noisy physical channel simulator (`channel.c`) injecting bit flips, byte drops, and bit flips, dropped bytes, and inserted garbage bytes.
* **What was tested & compared**:
  * Streamed 100,000 floating-point frames across a corrupted channel to verify synchronization recovery and data integrity.
  * Compared **CRC-16** (`test_robust`) against a weak **8-bit XOR checksum** (`test_robust_xor`, compiled with `-DCHECK_XOR`), demonstrating that CRC-16 rejects all corrupted frames (0 false positives) while 8-bit XOR allows silent data corruption through.

## Part 2: UART Reception & Data Streaming (`demo_uart.c`)

* **What was implemented**:
  * A thread-based UART peripheral simulator (`uart_sim.c`) modeling real-time byte arrival at 115200 baud (87 µs/byte), hardware RX register overwrites (overruns), and interrupt triggering.
  * A lock-free Single-Producer Single-Consumer (SPSC) ring buffer (`ring_buffer.c`) using C11 atomics to decouple the minimal RX ISR from the main application loop.
* **What was tested & compared**:
  * Transmitted 1,000 frames (13,000 bytes) while the main loop executed 200 µs compute chunks (`do_work_chunk`) to simulate real-world CPU load.
  * Compared three reception strategies:
    1. **Blocking Polling**: High frame reception, but leaves 0% CPU time for application tasks.
    2. **Polling Between Work**: Frees the CPU, but causes severe RX register overruns and near-total frame loss whenever compute chunks exceed the 87 µs byte arrival time.
    3. **Interrupt + Ring Buffer**: Achieves 100% frame reception with zero overruns while keeping ~99% of CPU capacity available for computation.

## Build and Run (Linux / WSL)

Requires `gcc` (C11, `-pthread`) and `make`.

```bash
make            # Build all unit tests and demo executables
make run        # Run all tests and demos in order
make clean      # Remove compiled binaries
```