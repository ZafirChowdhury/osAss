## 1. Operating System Concepts
This project demonstrated two fundamental OS concepts: the distinction between User/Kernel space and the utility of Dynamic Kernel Extensions.

### A. Kernel Space vs. User Space
Modern operating systems segregate memory into two distinct spaces to ensure stability and security:

* **User Space:** This is where normal applications (web browsers, text editors, C programs using `printf`) execute. These applications have restricted access to hardware and memory. If a user application crashes, the OS can terminate it without affecting the rest of the system.
* **Kernel Space:** This is where the core of the OS executes (including the module written in this project). Code here runs with the highest privileges (often called "Ring 0" on x86 architectures).

**Significance in this project:** Because the module runs in kernel space, it cannot use standard C libraries like `<stdio.h>`. Instead, it uses kernel-specific headers (`<linux/kernel.h>`) and functions like `printk()` to log messages to the kernel ring buffer. Errors here are critical; a bug in this module could crash the entire operating system (Kernel Panic).

### B. Dynamic Kernel Extensions (Loadable Kernel Modules)
Linux is a monolithic kernel, but it supports **Loadable Kernel Modules (LKMs)**. This allows functionality to be added to or removed from the kernel at runtime without needing to recompile the entire kernel or reboot the machine.

**Implementation:** This was demonstrated using the `insmod` command to link the compiled `.ko` file into the running kernel and `rmmod` to unlink it. This mechanism is commonly used for device drivers (e.g., loading a driver when a USB device is plugged in).

---

## 2. Implementation Details
The project was implemented in C, utilizing the Linux Kernel API.

* **Entry and Exit Points:** The module defines two primary functions:
    * `simple_init()`: Registered via `module_init()`, this function is triggered when `insmod` is called. It prints the loading message and the `GOLDEN_RATIO_PRIME`.
    * `simple_exit()`: Registered via `module_exit()`, this function is triggered when `rmmod` is called. It calculates the GCD and prints the removal message.
* **Kernel Headers:**
    * `<linux/hash.h>` was included to access the constant `GOLDEN_RATIO_PRIME`.
    * `<linux/gcd.h>` was included to utilize the kernel's internal `gcd()` function for calculating the greatest common divisor.
* **The Makefile:** A specialized Makefile was created to invoke the Linux kernel build system (`/lib/modules/$(uname -r)/build`), ensuring the module was compiled against the headers matching the currently running kernel version.

---

## 3. Challenges Faced
During the development process, I encountered and resolved specific strict compilation warnings that are unique to kernel development:

**1. Makefile Complexity**
* **Issue:** I found the Makefile syntax difficult to understand, specifically the command required to invoke the kernel build system.
* **Resolution:** I used AI tools to break down the command structure and explain how the build system enters the kernel source directory before compiling my local files.

**2. Missing Prototypes**
* **Issue:** The compiler warned about missing prototypes for the `init` and `exit` functions.
* **Resolution:** I added the `static` keyword to both functions (e.g., `static int simple_init`). This restricts their scope to the local file, which is best practice for kernel functions.

---

## 4. Testing Procedures
The following steps were taken to verify the correctness of the module:

1.  **Compilation:** Ran `make` and verified that the `simple_kernel_module.ko` file was generated without errors.
2.  **Loading:** Executed `sudo insmod simple_kernel_module.ko`.
3.  **Verification:** Ran `lsmod | grep simple` to confirm the module was listed in the active kernel modules.
4.  **Log Check:** Ran `sudo dmesg` and observed the output: `Golden Ratio Prime: [Value]`.
5.  **Unloading:** Executed `sudo rmmod simple_kernel_module`.
6.  **Verification:** Ran `sudo dmesg` again to check the exit logs.
7.  **Result:** The log successfully displayed `GCD of 3700 and 24: 4`, confirming the math function worked correctly before the module unloaded.
8.  **Cleanup:** Ran `make clean` to remove all binary and temporary build files.