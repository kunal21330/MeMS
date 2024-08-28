# MeMS (Memory Management System)

## Overview

MeMS is a custom memory management system implemented in C that utilizes system calls `mmap` and `munmap` for memory allocation and deallocation. It manages memory using a custom free list structure and provides functions for memory allocation, deallocation, and statistics reporting.

## Table of Contents

- [Problem Statement](#problem-statement)
- [Constraints and Requirements](#constraints-and-requirements)
- [Free List Structure](#free-list-structure)
- [Function Implementations](#function-implementations)
- [Usage](#usage)
- [Submission Guidelines](#submission-guidelines)
- [Notes](#notes)

## Problem Statement

Implemented a custom memory management system (MeMS) using the C programming language. MeMS should utilize `mmap` for memory allocation and `munmap` for deallocation. The system must adhere to specific constraints and requirements.

## Constraints and Requirements

1. **Memory Management Calls**: Use only `mmap` and `munmap` for memory management. The use of other memory management functions like `malloc`, `calloc`, `free`, and `realloc` is strictly prohibited.
2. **Page Size**: Request memory in multiples of the system's `PAGE_SIZE`, determined using `getconf PAGE_SIZE`. For most Linux systems, this is 4096 bytes (4 KB).
3. **Deallocation**: Deallocate memory in multiples of `PAGE_SIZE` using `munmap`.
4. **Memory Allocation**: Allocate memory in multiples of `PAGE_SIZE`, but only provide the requested size to the user program.
5. **Free List Structure**: Maintain a free list with a doubly linked list structure to track memory segments as either `PROCESS` or `HOLE`.
6. **Virtual and Physical Addresses**: Manage and translate between MeMS virtual addresses and MeMS physical addresses.

## Free List Structure

The free list is a doubly linked list with the following features:

- **Main Chain**: Represents memory regions requested from the OS. Each node in the main chain points to a sub-chain.
- **Sub-Chain**: Represents segments of memory within the main chain node. Segments can be of type `PROCESS` (allocated) or `HOLE` (free).

## Function Implementations

### `void mems_init()`

Initializes the MeMS system, setting up the free list and global variables.

- **Input Parameter**: None
- **Returns**: Nothing

### `void mems_finish()`

Finalizes the MeMS system by unmapping allocated memory using `munmap`.

- **Input Parameter**: None
- **Returns**: Nothing

### `void* mems_malloc(size_t size)`

Allocates memory of the specified size. Uses existing free segments or requests more memory if necessary.

- **Parameter**: `size` - The size of the memory to allocate.
- **Returns**: MeMS Virtual Address

### `void mems_free(void* ptr)`

Frees the memory pointed to by `ptr` and updates the free list.

- **Parameter**: `ptr` - MeMS Virtual Address to free.
- **Returns**: Nothing

### `void mems_print_stats()`

Prints statistics about the memory managed by MeMS, including total mapped pages and unused memory.

- **Input Parameter**: None
- **Returns**: Nothing

### `void* mems_get(void* v_ptr)`

Returns the MeMS physical address corresponding to the given MeMS virtual address.

- **Parameter**: `v_ptr` - MeMS Virtual Address.
- **Returns**: MeMS Physical Address

## Usage

1. Clone the repository:
    ```bash
    git clone https://github.com/kunal21330/MeMS.git
    ```

2. Implement the functions as described in the `MeMS-Skeleton-code` provided.

3. Test the implementation with different scenarios to ensure correctness.


---

For more details, refer to the assignment template provided in the [GitHub repository](https://github.com/kunal21330/MeMS).
