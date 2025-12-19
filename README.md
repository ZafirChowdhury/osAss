## 
```bash
# Steps to run the project

# A
sudo apt update
sudo apt install build-essential

gcc collatz.c -o output
./output

# B
sudo apt update
sudo apt install build-essential
sudo apt install libssl-dev

gcc b.c -o b -lpthread -lssl -lcrypto
./output file_path

# C (Make sure python is installed)
python3 ./mlq_scheduler.py

# E (Make sure python is installed)
python3 ./dining_philosophers.py

# Optional

# A (build essential for the specific kernel i am running)
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)

# Compile the modual (check if .ko file exitts)
make

# Clear log
sudo dmesg -c

# Load the modual
sudo insmod simple_kernel_module.ko

# Chek if modual is loaded
lsmod | grep simple

# Look for the kernel modual start message (Golden Ratio Prime)
sudo dmesg

# Removing the kernel modual
sudo rmmod simple_kernel_module

# Look for kernel modual end message (GDC)
sudo dmesg

# Clean up build artefacts (opsonal)
make clean
