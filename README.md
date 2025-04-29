# Operating Systems Project 3  
**CS3113 – Introduction to Operating Systems – Spring 2025**

## Project Description  
This project simulates advanced process and memory management in a simplified operating system. It builds upon the features of Projects One and Two by introducing a memory allocation mechanism based on dynamic partitioning with first-fit strategy, and incorporates a New Job Queue to manage memory contention.

Processes are loaded from the NewJobQueue into memory only if there is sufficient contiguous space. If space is unavailable, memory coalescing (merging of adjacent free blocks) is attempted. Upon process termination, memory is released and made available for new jobs.

## Key Enhancements Introduced  
1. **NewJobQueue Management**  
   - Jobs enter the NewJobQueue instead of the ReadyQueue.
   - Jobs are loaded only if sufficient contiguous memory is available.

2. **Dynamic Memory Allocation and Tracking**  
   - Memory is tracked as a list of blocks (free or occupied), each with a starting address, size, and process ID.
   - Initially, memory is represented as a single large free block.

3. **Memory Coalescing**  
   - If no block is large enough to fit a job, adjacent free blocks are merged.
   - The NewJobQueue is then re-evaluated for potential job loading.

4. **Job Termination and Memory Release**  
   - Terminated jobs release memory, marking the block as free.
   - This enables subsequent job loading attempts from the NewJobQueue.

5. **Instruction Execution and CPU Scheduling**  
   - Maintains time slicing, context switching, and I/O handling from Project Two.
   - Tracks job states: NEW, READY, RUNNING, I/O WAITING, and TERMINATED.

## Files Included  
- `CS3113_Project3.cpp`: Full C++ implementation of the process and memory management simulation.  
- `CS3113-Spring-2025-ProjectThree.pdf`: Official specification document.  
- `README.md`: Project documentation and instructions.

## Input Format  
The input is read via standard input redirection and follows this format:
1. Integer: Total main memory size  
2. Integer: CPU time slice (maximum cycles a process may run before timeout)  
3. Integer: Context switch time (added to the global clock per switch)  
4. Integer: Number of processes  
5. For each process:
   - `processID maxMemoryNeeded numInstructions`  
   - Sequence of instructions formatted as:

### Supported Instruction Set  
| Opcode | Instruction | Format                       | Description                      |
|--------|-------------|------------------------------|----------------------------------|
| 1      | Compute     | `1 <iterations> <cycles>`    | Increments clock and CPU usage  |
| 2      | Print       | `2 <cycles>`                 | Triggers I/O interrupt           |
| 3      | Store       | `3 <value> <address>`        | Stores value in memory           |
| 4      | Load        | `4 <address>`                | Loads value into register        |

## Compilation and Execution  
To compile and run the simulation:
```bash
g++ CS3113_Project3.cpp -o os_project3
./os_project3 < input.txt
