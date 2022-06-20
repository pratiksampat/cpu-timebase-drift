A Module-based script to test for any TimeBase anomalies or drifts from one CPU to another over time

Inspired by the userspace program by Anton Blanchard, this program works in the kernel therefore giving us a closer time-view of the system.

To build, hit `make`

To run, enable running the test in the following debugfs directory

```
echo 1 > /sys/kernel/debug/tb_test/tb_test_activate
```

The result will be logged at
```
/sys/kernel/debug/tracing/trace
```
In the case their is a TB drift, the result will also be printed in dmesg ring buffer.

Disable the experiment logging using

```
echo 0 > /sys/kernel/debug/tb_test/tb_test_activate
```
