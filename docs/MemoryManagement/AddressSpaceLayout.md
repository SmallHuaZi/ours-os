# Address Space Layout Overview
## 1. Map 

## 2. Physical memory layout 
1. __DMA__: physical address [0x0000, 0xFFFF].
2. __DMA32__: physical address [].
3. __Normal__: .
4. __Super__: Physical memory range which exceeds the maximum addressing space.

## 2. Virtual memory layout 
1. Kernel code region: .
2. Kernel data region:.
3. Kernel bss region:.
4. __Page map region__: Usually it is maximum virtual memory address minus the memory size required by all [__PmFrame__](./Classes.md#PmFrame).
