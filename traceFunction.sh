#!/bin/sh

sudo su

mount -t tracefs nodev /sys/kernel/tracing

echo 0 > tracing_on
echo 0 > tracing_thresh

#echo SyS_sched_getattr > set_ftrace_filter
echo SyS_sched_getattr > set_graph_function
echo function_graph > current_tracer
echo funcgraph-duration > trace_options

echo 100 > buffer_size_kb

echo 1 > tracing_on







cat trace | grep SyS_sched_getattr | sed -e "s/ \+/ /g" | cut -f 3 -d " " | 
