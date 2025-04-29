#include <iostream>
#include <queue>
#include <list>
#include <vector>
#include <tuple>
#include <algorithm>

using namespace std;

struct PCB {
    int processID;
    // Process states: 0 = new, 1 = ready, 2 = running, 3 = i/o waiting, 4 = terminated
    int state;
    int programCounter;
    int instructionBase;
    int dataBase;
    int memoryLimit;     // number of words needed (excluding PCB overhead)
    int cpuUsed;
    int registerValue;
    int maxMemoryNeeded; // as given by input (e.g., for process 1: 231)
    int mainMemoryBase;  // assigned start address in mainMemory
    vector<int> logicalMemory; // instructions and associated data
};

struct IORequest {
    int startAddress;   // block start address of the process in mainMemory
    int dataPointer;    // pointer to the next data word to be used
    int exitTime;       // global time when I/O completes
};

struct ReadyItem {
    int startAddress;   // starting address in mainMemory
    int dataPointer;    // pointer to next data word
};

void printReadyQueue(const queue<ReadyItem> &readyQueue) {
    queue<ReadyItem> temp = readyQueue;
    while (!temp.empty()) {
        ReadyItem item = temp.front();
        temp.pop();
        cout << "ReadyItem: StartAddress = " << item.startAddress
             << ", DataPointer = "    << item.dataPointer << endl;
    }
}

void printIOQueue(const queue<IORequest> &ioQueue) {
    queue<IORequest> temp = ioQueue;
    while (!temp.empty()) {
        IORequest item = temp.front();
        temp.pop();
        cout << "IORequest: StartAddress = " << item.startAddress
             << ", ExitTime = "       << item.exitTime << endl;
    }
}

void printNewJobQueue(const queue<PCB> &newJobQueue) {
    queue<PCB> temp = newJobQueue;
    while (!temp.empty()) {
        PCB job = temp.front();
        temp.pop();
        cout << "PCB: ProcessID = " << job.processID 
             << ", State = "       << job.state << endl;
    }
}

void printQueues(const queue<PCB> &newJobQueue,
                 const queue<ReadyItem> &readyQueue,
                 const queue<IORequest> &ioQueue) {
    cout << "New Job Queue:" << endl;
    printNewJobQueue(newJobQueue);
    cout << "Ready Queue:" << endl;
    printReadyQueue(readyQueue);
    cout << "IO Queue:" << endl;
    printIOQueue(ioQueue);
}

struct MemoryBlock {
    int processID;       // -1 if free, otherwise the process ID
    int startAddress;    // starting address in mainMemory
    int size;            // block size in words
};

bool operator==(const MemoryBlock &a, const MemoryBlock &b) {
    return a.processID   == b.processID
        && a.startAddress == b.startAddress
        && a.size         == b.size;
}

class MemoryManager {
public:
    MemoryManager(int maxMem)
        : maxMemory(maxMem)
    {
        mainMemory = new int[maxMemory];
        for (int i = 0; i < maxMemory; i++) {
            mainMemory[i] = -1;
        }
        memList.push_back({-1, 0, maxMemory});
    }
    ~MemoryManager() {
        delete[] mainMemory;
    }
    int* getMainMemory() { return mainMemory; }
    
    void printMainMemory() {
        for (int i = 0; i < maxMemory; i++) {
            cout << i << " : " << mainMemory[i] << endl;
        }
    }
    
    void loadJobs(queue<PCB> &newJobQueue, queue<ReadyItem> &readyQueue) {
        bool loadedSomething = true;
        while (loadedSomething && !newJobQueue.empty()) {
            loadedSomething = false;
            PCB &job = newJobQueue.front();
            int neededSize = 10 + job.memoryLimit; // 10-word overhead
            auto it = findFirstFit(neededSize);
            if (it == memList.end()) {
                cout << "Insufficient memory for Process " 
                     << job.processID << ". Attempting memory coalescing." 
                     << endl;
                coalesceFreeBlocks();
                it = findFirstFit(neededSize);
                if (it == memList.end()) {
                    cout << "Process " << job.processID
                         << " waiting in NewJobQueue due to insufficient memory."
                         << endl;
                    return;
                } else {
                    cout << "Memory coalesced. Process " << job.processID
                         << " can now be loaded." << endl;
                    allocateBlock(*it, job);
                    cout << "Process " << job.processID 
                         << " loaded into memory at address "
                         << it->startAddress << " with size " << neededSize
                         << "." << endl;
                    writeProcessToMemory(job);
                    ReadyItem newReady;
                    newReady.startAddress = job.mainMemoryBase;
                    newReady.dataPointer = job.dataBase;
                    readyQueue.push(newReady);
                    newJobQueue.pop();
                    loadedSomething = true;
                }
            } else {
                allocateBlock(*it, job);
                cout << "Process " << job.processID
                     << " loaded into memory at address "
                     << it->startAddress << " with size " << neededSize
                     << "." << endl;
                writeProcessToMemory(job);
                ReadyItem newReady;
                newReady.startAddress = job.mainMemoryBase;
                newReady.dataPointer = job.dataBase;
                readyQueue.push(newReady);
                newJobQueue.pop();
                loadedSomething = true;
            }
        }
    }
    
    void freeProcess(int pid) {
        for (auto &blk : memList) {
            if (blk.processID == pid) {
                int start = blk.startAddress;
                int end = start + blk.size - 1;
                blk.processID = -1;
                for (int addr = start; addr < start + blk.size; addr++) {
                    mainMemory[addr] = -1;
                }
                cout << "Process " << pid 
                     << " terminated and released memory from "
                     << start << " to " << end << "." << endl;
            }
        }
    }

    void printMemoryBlock(const MemoryBlock &block) {
        cout << "Process ID: " << block.processID
             << ", Start Address: " << block.startAddress
             << ", Size: " << block.size << endl;
    }
    
    void printMemoryBlocks() {
        for (const auto &block : memList) {
            printMemoryBlock(block);
        }
    }
    
private:
    int maxMemory;
    int* mainMemory;
    list<MemoryBlock> memList;
    
    list<MemoryBlock>::iterator findFirstFit(int neededSize) {
        for (auto it = memList.begin(); it != memList.end(); ++it) {
            if (it->processID == -1 && it->size >= neededSize) {
                return it;
            }
        }
        return memList.end();
    }
    
    void allocateBlock(MemoryBlock &block, PCB &job) {
        int neededSize = 10 + job.memoryLimit;
        job.mainMemoryBase = block.startAddress;
        job.state = 1; // ready
        job.instructionBase = block.startAddress + 10;
        // last element in job.logicalMemory is #instructions
        job.dataBase = job.instructionBase + job.logicalMemory[job.logicalMemory.size() - 1];
        block.processID = job.processID;
        if (block.size > neededSize) {
            MemoryBlock newFree;
            newFree.processID = -1;
            newFree.startAddress = block.startAddress + neededSize;
            newFree.size = block.size - neededSize;
            block.size = neededSize;
            for (auto itr = memList.begin(); itr != memList.end(); ++itr) {
                if (&(*itr) == &block) {
                    ++itr;
                    memList.insert(itr, newFree);
                    break;
                }
            }
        }
    }
    
    void coalesceFreeBlocks() {
        bool merged = true;
        while (merged) {
            merged = false;
            for (auto it = memList.begin(); it != memList.end();) {
                auto nextIt = next(it);
                if (nextIt == memList.end()) break;
                if (it->processID == -1 && nextIt->processID == -1) {
                    it->size += nextIt->size;
                    it = memList.erase(nextIt);
                    merged = true;
                } else {
                    ++it;
                }
            }
        }
    }
    
    void writeProcessToMemory(const PCB &job) {
        int start = job.mainMemoryBase;
        // PCB metadata in first 10 words
        mainMemory[start + 0] = job.processID;
        mainMemory[start + 1] = job.state;
        mainMemory[start + 2] = job.programCounter;
        mainMemory[start + 3] = job.instructionBase;
        mainMemory[start + 4] = job.dataBase;
        mainMemory[start + 5] = job.memoryLimit;
        mainMemory[start + 6] = job.cpuUsed;
        mainMemory[start + 7] = job.registerValue;
        mainMemory[start + 8] = job.maxMemoryNeeded;
        mainMemory[start + 9] = job.mainMemoryBase;
        for (int i = 0; i < (int)job.logicalMemory.size() - 1; i++) {
            mainMemory[start + 10 + i] = job.logicalMemory[i];
        }
    }
};
class CPU {
public:
    CPU(int timeSlice, int numProcs)
        : cpuAllocated(timeSlice), globalClock(0)
    {
        startTimes.resize(numProcs, -1);
    }
    
    // Execute instructions for the process at startAddress.
    tuple<bool, int> executeCPU(int startAddress,
                                int dataPointer,
                                int* mainMemory,
                                queue<ReadyItem> &readyQueue,
                                queue<IORequest> &ioQueue,
                                MemoryManager &memManager)
    {
        int pid = mainMemory[startAddress + 0];
        int state = mainMemory[startAddress + 1];
        int pc = mainMemory[startAddress + 2];
        int instrBase = mainMemory[startAddress + 3];
        int db = mainMemory[startAddress + 4];
        int memLimit = mainMemory[startAddress + 5];
        int cpuUsed = mainMemory[startAddress + 6];
        int regVal = mainMemory[startAddress + 7];
        int maxMemNeed = mainMemory[startAddress + 8];
        int mmBase = mainMemory[startAddress + 9];
        
        if (pc == 0) {
            pc = instrBase;
        }
        mainMemory[startAddress + 2] = pc;
        state = 2; // running
        mainMemory[startAddress + 1] = state;

        // If first time, mark start time
        if (pid - 1 >= 0 && pid - 1 < (int)startTimes.size()
            && startTimes[pid - 1] == -1) {
            startTimes[pid - 1] = globalClock;
        }
        
        bool ioFlag = false;
        bool terminated = false;
        int sliceUsed = 0;

        while (pc < db && !ioFlag) {
            int instruction = mainMemory[pc];
            switch (instruction) {
                case 1: { 
                    cout << "compute" << endl;
                    int dummy = mainMemory[dataPointer];
                    dataPointer++;
                    int cycles = mainMemory[dataPointer];
                    dataPointer++;
                    cpuUsed += cycles;
                    sliceUsed += cycles;
                    globalClock += cycles;
                    mainMemory[startAddress + 6] = cpuUsed;
                    pc++;
                    mainMemory[startAddress + 2] = pc;
                    break;
                }
                case 2: { // IO
                    int cycles = mainMemory[dataPointer];
                    dataPointer++;
                    cpuUsed += cycles;
                    mainMemory[startAddress + 6] = cpuUsed;
                    pc++;
                    mainMemory[startAddress + 2] = pc;
                    int exitTime = globalClock + cycles;
                    ioFlag = true;
                    mainMemory[startAddress + 1] = 3; // i/o waiting
                    ioQueue.push({startAddress, dataPointer, exitTime});
                    cout << "Process " << pid
                         << " issued an IOInterrupt and moved to the IOWaitingQueue."
                         << endl;
                    break;
                }
                case 3: { // store
                    int value = mainMemory[dataPointer];
                    dataPointer++;
                    regVal = value;
                    mainMemory[startAddress + 7] = regVal;// store regVal
                    int address = mainMemory[dataPointer];
                    dataPointer++;
                    address += (mmBase + 10);
                    if (address >= db && address < (startAddress + 10 + memLimit)) {
                        cout << "stored" << endl;
                    } else {
                        cout << "store error!" << endl;
                    }
                    cpuUsed++;
                    sliceUsed++;
                    globalClock++;
                    mainMemory[startAddress + 6] = cpuUsed;
                    pc++;
                    mainMemory[startAddress + 2] = pc;
                    break;
                }
                case 4: { // load
                    int address = mainMemory[dataPointer];
                    dataPointer++;
                    address += (mmBase + 10);
                    if (address >= db && address < (startAddress + 10 + memLimit)) {
                        int value = mainMemory[address];
                        regVal = value; // <--- store loaded value
                        mainMemory[startAddress + 7] = regVal;
                        cout << "loaded" << endl;
                    } else {
                        cout << "load error!" << endl;
                    }
                    cpuUsed++;
                    sliceUsed++;
                    globalClock++;
                    mainMemory[startAddress + 6] = cpuUsed;
                    pc++;
                    mainMemory[startAddress + 2] = pc;
                    break;
                }
                default:
                    cout << "Unknown instruction code: " << instruction << endl;
                    pc++;
                    break;
            }

            // Time-slice check
            if (!ioFlag && sliceUsed >= cpuAllocated && pc < db) {
                mainMemory[startAddress + 1] = 1; // ready
                readyQueue.push({startAddress, dataPointer});
                cout << "Process " << pid
                     << " has a TimeOUT interrupt and is moved to the ReadyQueue."
                     << endl;
                return make_tuple(false, pid);
            }
        }

        // If we exit the loop with no IO and pc >= db, process terminates
        if (!ioFlag && pc >= db) {
            terminated = true;
            pc = instrBase - 1;
            mainMemory[startAddress + 2] = pc;
            mainMemory[startAddress + 1] = 4; // terminated
            cout << "Process ID: " << pid << endl;
            cout << "State: TERMINATED" << endl;
            cout << "Program Counter: " << mainMemory[startAddress + 2] << endl;
            cout << "Instruction Base: " << mainMemory[startAddress + 3] << endl;
            cout << "Data Base: " << mainMemory[startAddress + 4] << endl;
            cout << "Memory Limit: " << mainMemory[startAddress + 5] << endl;
            cout << "CPU Cycles Used: " << mainMemory[startAddress + 6] << endl;
            cout << "Register Value: " << mainMemory[startAddress + 7] << endl;
            cout << "Max Memory Needed: " << mainMemory[startAddress + 8] << endl;
            cout << "Main Memory Base: " << mainMemory[startAddress + 9] << endl;
            int rawConsumed = globalClock - startTimes[pid - 1];
            cout << "Total CPU Cycles Consumed: " 
                 << rawConsumed << endl;
            cout << "Process " << pid
                 << " terminated. Entered running state at: "
                 << startTimes[pid - 1]
                 << ". Terminated at: " << globalClock
                 << ". Total Execution Time: " << rawConsumed
                 << "." << endl;
        }
        return make_tuple(terminated, pid);
    }
    
    int getGlobalClock() const { return globalClock; }
    void addContextSwitch(int cst) { globalClock += cst; }
    
private:
    int cpuAllocated;
    int globalClock;
    vector<int> startTimes;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int maxMemory, cpuAllocated, contextSwitchTime, numProcesses;
    cin >> maxMemory >> cpuAllocated >> contextSwitchTime;
    cin >> numProcesses;
    
    MemoryManager memManager(maxMemory);
    CPU cpu(cpuAllocated, numProcesses);
    queue<PCB> newJobQueue;
    queue<ReadyItem> readyQueue;
    queue<IORequest> ioQueue;
    
    // Read processes into NewJobQueue
    for (int i = 0; i < numProcesses; i++) {
        PCB job;
        cin >> job.processID;
        job.state = 0;
        job.programCounter = 0;
        job.cpuUsed  = 0;
        job.registerValue  = 0;
        job.instructionBase = 10;
        
        cin >> job.maxMemoryNeeded;
        job.memoryLimit = job.maxMemoryNeeded;
        
        int numInstructions;
        cin >> numInstructions;
        job.dataBase = job.instructionBase + numInstructions;
        for (int j = 0; j < numInstructions; j++) {
            job.logicalMemory.push_back(0);
        }
        for (int j = 0; j < numInstructions; j++) {
            int instr;
            cin >> instr;
            job.logicalMemory[j] = instr;
            if (instr % 2 == 0) {
                int d1; cin >> d1;
                job.logicalMemory.push_back(d1);
            } else {
                int d1, d2;
                cin >> d1 >> d2;
                job.logicalMemory.push_back(d1);
                job.logicalMemory.push_back(d2);
            }
        }
        job.logicalMemory.push_back(numInstructions);
        newJobQueue.push(job);
    }
    
    memManager.loadJobs(newJobQueue, readyQueue);
    memManager.printMainMemory();
    
    // Main simulation
    while (!newJobQueue.empty() || !readyQueue.empty() || !ioQueue.empty()) {
        if (!readyQueue.empty()) {
            cpu.addContextSwitch(contextSwitchTime);
            // context switch
            ReadyItem item = readyQueue.front();
            readyQueue.pop();
            cout << "Process "
                 << memManager.getMainMemory()[item.startAddress]
                 << " has moved to Running." << endl;
            auto [terminated, pid] = cpu.executeCPU(item.startAddress,
                                                    item.dataPointer,
                                                    memManager.getMainMemory(),
                                                    readyQueue,
                                                    ioQueue,
                                                    memManager);
            if (terminated) {
                memManager.freeProcess(pid);
                memManager.loadJobs(newJobQueue, readyQueue);
            }
        } else {
            // No ready items
            if (!ioQueue.empty()) {
                int currentTime = cpu.getGlobalClock();
                vector<int> exitTimes;
                {
                    queue<IORequest> temp = ioQueue;
                    while (!temp.empty()) {
                        exitTimes.push_back(temp.front().exitTime);
                        temp.pop();
                    }
                }
                int earliest = *min_element(exitTimes.begin(), exitTimes.end());
            //  debug  cout << "Earliest I/O completion at: " << earliest << endl;
                if (earliest > currentTime) {
                    cpu.addContextSwitch(contextSwitchTime);
                }
            } else {
                // try to load new jobs again
                memManager.loadJobs(newJobQueue, readyQueue);
            }
        }

        // Process IO completions
        {
            int currentTime = cpu.getGlobalClock();
            int n = ioQueue.size();
            for (int i = 0; i < n; i++) {
                IORequest req = ioQueue.front();
                ioQueue.pop();
                if (currentTime >= req.exitTime) {
                    int pid = memManager.getMainMemory()[req.startAddress + 0];
                    memManager.getMainMemory()[req.startAddress + 1] = 1;
                    readyQueue.push({req.startAddress, req.dataPointer});
                    cout << "print" << endl;
                    cout << "Process " << pid
                         << " completed I/O and is moved to the ReadyQueue."
                         << endl;
                } else {
                    ioQueue.push(req);
                }
            }
        }
    }

    // Final context switch
//    cout << cpu.getGlobalClock() << endl;
    cpu.addContextSwitch(contextSwitchTime);
    int finalClock = cpu.getGlobalClock();
    cout << "Total CPU time used: " << finalClock << "."<< endl;
    return 0;
}
