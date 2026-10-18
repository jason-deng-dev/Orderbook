# The Basics of Profiling - Mathieu Ropert - CppCon 2021
https://www.youtube.com/watch?v=dToaepIXW4s&t=107s

**Profiling vs Optimization**
- profilers are one of the tools that can be used during optimizaation iteration cycle
- better used to investigate where to optimize

**Profiler usage**
- used to identify hotspots & bottlenecks
- visualize execution timeline
- collect & compute metrics

**Sample profiling**
- attach to program, periodically interrupt and record the stack trace
- sampling frequency is customizable
- results are statistical averages
- only needs to be able to read stack trace
- inlined functions are usually invisible
- Example tool:vTune

**Instrumentation Profiling**
- Add code hooks to explicitly record metrics
- Can provide both averages and exact breakdown by execution frame
- not affected by inlining or statistical anomalies
- Example tool:Optick
- Requires programmer to add collection macros in tactical places in code
