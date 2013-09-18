# fdtrace
## Usage:

    fdtrace COMMAND

## Description
`fdtrace` is a simple utility that displays the file descriptors used by a process. It prints a list of current file descriptor links every time a ~write~ system call is made. 

## Requirements
`fdtrace` makes use of the `ptrace` system call and the `/proc` file system.

## Issues
Has not yet been tested on 32 bit architecture
