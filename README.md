# serial_link

A small, layered C project that answers questions 1 and 2 of the Condor Pacific
home assignment with working, tested code instead of text alone.

```
apps / demos   test_robust.c (Q2)   demo_uart.c (Q1)
protocol       crc16.c   frame.c   parser.c
drivers        ring_buffer.c
HAL            uart_sim.c + channel.c   (PC simulation - the only layer that changes on hardware)
```

## Build and run (Linux / WSL)

    make            # build everything
    make run        # run all tests and demos
    make clean

The small programs in `try/` were used to check each step while building the
project. Each file starts with a comment that shows how to build it.

Requires gcc and make.
