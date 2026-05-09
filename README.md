# OS Course Assignments — Autumn 2025

University Operating Systems course assignments covering process creation, multithreaded synchronization, CPU scheduling, concurrency, and kernel programming. Written in C and Python.

The full assignment spec is in `OS Project - Autumn 2025.pdf`.

---

## Mandatory A — Collatz Conjecture (`collatz.c`)

**Task:** Given a positive integer as a command-line argument, compute and print the Collatz sequence using a child process. The sequence applies the rule: if n is even, divide by 2; if odd, multiply by 3 and add 1 — repeating until n reaches 1.

**Solution:** Used `fork()` to create a child process. The child runs the Collatz loop and prints each value in the sequence. The parent calls `wait(NULL)` to block until the child finishes, preventing it from becoming a zombie process.

**What I learned:**
- How `fork()` works and how parent and child processes diverge after the call
- Using `wait()` to synchronize the parent with a child process
- The difference between process-based concurrency (`fork`) and thread-based concurrency (`pthreads`)
- Using `perror()` to print system-level error messages

```bash
gcc collatz.c -o output
./output <number>
```

---

## Mandatory B — Multithreaded MD5 Directory Hasher (`md5Hash.c`)

**Task:** Recursively walk a directory, compute the MD5 hash of every file found, and print each filename alongside its hash — using multiple threads to do the hashing concurrently.

**Solution:** Implemented the classic **producer-consumer pattern** with a circular buffer of size 10. The main thread acts as the producer: it recursively traverses the directory tree using `opendir`/`readdir` and writes file paths into the shared buffer. Eight consumer threads read from the buffer and compute each file's MD5 hash using OpenSSL. Synchronization is handled with two semaphores (`empty` and `full`) to track buffer capacity and availability, and a mutex to protect the buffer indices during reads and writes. A `done` flag signals consumers to exit once the producer finishes, with extra `sem_post` calls to wake any threads blocked on an empty buffer.

**What I learned:**
- The producer-consumer problem and how to solve it with semaphores + a mutex
- How a circular buffer works and why index arithmetic (`% N`) is used
- The difference between semaphores (signalling capacity) and mutexes (protecting shared state)
- Using the OpenSSL `MD5_CTX` API to hash file contents incrementally
- Graceful thread shutdown with a `done` flag

```bash
gcc md5Hash.c -o output -lpthread -lssl -lcrypto
./output <directory_path>
```

---

## Mandatory C — Multi-Level Queue Scheduler (`mlq_scheduler.py`)

**Task:** Simulate a Multi-Level Queue (MLQ) CPU scheduling algorithm with three priority queues, each using a different scheduling policy, and support preemption when a higher-priority process arrives.

**Solution:** Implemented a tick-by-tick simulation in Python. Three queues are defined: Queue 0 (high priority, Round Robin, quantum=2), Queue 1 (medium priority, Round Robin, quantum=4), and Queue 2 (low priority, FCFS). Each tick, the scheduler checks for new process arrivals, determines the highest non-empty queue, and preempts the current process if a higher-priority queue has work. The simulation tracks response time, waiting time, turnaround time, CPU utilization, and throughput, and prints a full Gantt chart at the end.

**What I learned:**
- How MLQ scheduling works and why different queues use different policies
- The mechanics of preemption — interrupting a running process when a higher-priority one arrives
- How Round Robin's time quantum affects fairness and response time
- Calculating scheduling metrics: turnaround time, waiting time, response time, CPU utilization, and throughput
- Simulating concurrent behaviour sequentially using a tick-based loop

```bash
python3 ./mlq_scheduler.py
```

---

## Mandatory E — Dining Philosophers (`dining_philosophers.py`)

**Task:** Implement the classic Dining Philosophers problem — five philosophers sharing five forks, each needing two forks to eat — without causing deadlock or starvation.

**Solution:** Used the **Monitor pattern** rather than simple locks. A `DiningServer` class holds a single global lock and one `threading.Condition` variable per philosopher. When a philosopher wants to eat, it sets its state to `HUNGRY` and calls `test()`, which checks whether both neighbours are not currently eating. If the check fails, the philosopher waits on its own condition variable. When a philosopher finishes eating, it calls `test()` on both its neighbours, waking them up if they can now eat. This avoids deadlock because philosophers never hold one fork while waiting for the other.

**What I learned:**
- Why naive mutex-based approaches cause deadlock in the Dining Philosophers problem
- The Monitor concurrency pattern and how it centralizes state management
- Using `threading.Condition` in Python for fine-grained per-thread waiting and signalling
- How `notify()` and `wait()` work within a condition variable
- The importance of checking state in a `while` loop (not `if`) to handle spurious wakeups

```bash
python3 ./dining_philosophers.py
```

---

## Optional A — Linux Kernel Module (`simple_kernel_module.c`)

**Task:** Write a loadable Linux kernel module that executes code when inserted into and removed from the running kernel, demonstrating the basics of kernel space programming.

**Solution:** Written in C using the Linux Kernel API. `simple_init()` is registered with `module_init()` and fires on `insmod` — it logs a loading message and prints the `GOLDEN_RATIO_PRIME` constant from `<linux/hash.h>`. `simple_exit()` is registered with `module_exit()` and fires on `rmmod` — it computes `gcd(3700, 24)` using the kernel's built-in `gcd()` function from `<linux/gcd.h>` and logs the result (4) before unloading. All output uses `printk()` instead of `printf()`, since standard C libraries are unavailable in kernel space. A custom Makefile invokes the kernel build system against the currently running kernel's headers.

**What I learned:**
- The difference between user space and kernel space — standard C libraries don't exist in the kernel, and a bug here can cause a kernel panic
- How Loadable Kernel Modules (LKMs) let you extend a monolithic Linux kernel at runtime without rebooting
- Using `printk()` and the kernel ring buffer (`dmesg`) for kernel-level logging
- How `module_init()` and `module_exit()` macros register entry and exit points
- Why kernel functions should be declared `static` — to limit their scope to the current file, which is best practice to avoid symbol collisions in the kernel namespace
- How the kernel Makefile build system works and how to compile a module against the running kernel's headers

```bash
sudo apt install build-essential linux-headers-$(uname -r)
make
sudo insmod simple_kernel_module.ko    # loads module — check: sudo dmesg
sudo rmmod simple_kernel_module        # unloads module — check: sudo dmesg
make clean
```
