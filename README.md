## 简介
这是一个玩具vmm和一个可以跑在vmm里面的一个小kernel， 因为刚开始接触qemu-kvm不久，然后找到了sparkler这个最简单的vmm。不过sparkler运行的kernel是在实模式下运行的16bit asm code， 我就尝试进行拓展，目前支持用c语言编写运行的kernel，而且可以运行在32bit protected mode或者64bit long mode中。

## Usage
`make` 
`cd build && ./vmm`
或者直接
`make run`

## TODO
想加入一个基本的内存管理，管理frame和page的映射

