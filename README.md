# procsplice
![](./procsplice-logo.png)    

Tool for working with memory of a running Linux process   

## Note

You must be able to ptrace the process you wish to operate on. If something like [yama](https://www.kernel.org/doc/Documentation/security/Yama.txt) is used then this may require root/CAP_SYS_PTRACE even for processes you own. 

(This tool doesn't use ptrace, rather it uses process_vm_readv / process_vm_writev but the access checks are the same) 

## Motivation

I wrote this tool to use in some scripts. Some of the actions that can be performed only make sense on a stopped process (SIGSTOP/SIGCONT etc) and may cause terrible things to happen otherwise! This tool isn't magic - memory permissions must allow you to perform write actions etc (for a hacky way to update memory permissions in another process (x86_64 only) see tools directory) 

## Stack Dumping

Dump the stack of a process to a file:

```
procsplice -sd -p 1234 -f /tmp/stack_1234
```

## Heap Dumping

Dump the heap of a process to a file:

```
procsplice -hd -p 1234 -f /tmp/heap_1234
```

## Code Dumping

Dump main process to a file: 

```
procsplice -cd -p 1234 -f /tmp/code_1234
```

## Arbitrary Dumping

Dump some part of process memory to a file: 

```
procsplice -ad -p 1234 -i 5562ad385000 -o 5562ad489000 -f /tmp/something_1234
```

## Stack Loading

Load file to stack:

```
procsplice -sl -p 1234 -f /tmp/stack_1234
```

## Heap Loading

Load file to heap:

```
procsplice -hl -p 1234 -f /tmp/heap_1234
```

## Code Loading

Load file to code:

```
procsplice -cl -p 1234 -f /tmp/code_1234
```

## Arbitrary Loading

Load file somewhere:

```
procsplice -al -p 1234 -i 5562ad385000 -o 5562ad489000 -f /tmp/lucky_something_1234
```
