#!/bin/sh

prefix=/home/hushouguo/packages/jemalloc-5.1.0/local
exec_prefix=/home/hushouguo/packages/jemalloc-5.1.0/local
libdir=${exec_prefix}/lib

LD_PRELOAD=${libdir}/libjemalloc.so.2
export LD_PRELOAD
exec "$@"
