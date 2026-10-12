# perf usage
```c
void RunPerf(){
  pid_t pid = fork();
  if (pid == 0) {
    const auto parentPid = std::to_string(getppid());
    std::cout<< "Running perf on parent process " << parentPid << std::endl;
    execlp("perf", "perf", "stat", "-", parentPid.c_str(), (char*)nullptr);
    throw std::runtime_error("execlp failed");
  }
}

void InitAndRunBenchmark() {
  InitBenchmark(); // might take long time!
  RunPerf();
  RunBenchmark();
}
```
- approach won't work for micro benchmark, thus we benchmark on NASDAQ_ITCH50 data fed to our benchmark through our Feed_Handler
- we start perf after the Init process since it might take a long time, thus distorting the accuracy of our performance results

```c
execlp("perf", "perf", "stat", "-", parentPid.c_str(), (char*)nullptr);
```
- replaces the current process (child) with a completely new program (perf)
- the child process stops being a copy of benchmark and becomes perf

|Argument| What it is  | Why|
|--|--|--|
"perf" (1st) | program to run | tells OS find perf in PATH and execute it
"perf" (2nd) | argv[0] | by convention, first argument to any C program is its own name
"stat" | argv[1] | tells perf to run in stat mode (count events)
"-p" | argv[2] | attach to PID
parentPid.c_str() | argv[3] | actual PID of benchmark
(char*)nullptr| terminator | tells execlp  (variadic function) this is the end of list

# experimental log
perform benchmark/perf on different versions of the Orderbook/search policies/data structure implementations to see the performance differences

On each run record a experiment log, to be able to compare authoriatively the effects our refactors/different approaches 

```
date:
git commit:
compiler + flags:
kernel + perf version:
PU model + topology + SMT state:
governor/frequency notes:
workload + seed + duration:
pinning:
perf stat command:
primary metric:
result (median + spread):
interpretation:
boundary/when this should not generalize:
```
