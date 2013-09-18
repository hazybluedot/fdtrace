# fdtrace
## Usage:

    fdtrace COMMAND

## Description
`fdtrace` is a simple utility that displays the file descriptors used by a process. Before each ~write~ system call the current file descriptor links are printed if they haven't changed since the last call to ~write~ (i.e. there wasn't a call to ~open~, ~close~, ~dup~ or ~fcntl~). 

## Requirements
`fdtrace` makes use of the `ptrace` system call and the `/proc` file system.

## Issues
- Has not yet been tested on 32 bit architecture
- Arguments printed for write system call are incorrect, apparently the RBX, RCX and RDX registers are not used in the same way the EBX, ECX and EDX registers are... or I'm reading them incorrectly.