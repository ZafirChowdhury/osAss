## Prerequisites & Setup
```bash

# A
# Update package list
sudo apt update

# Install GCC, Make, and standard build tools
sudo apt install build-essential

# Compile	
gcc a.c -o a

# Run
./a

# B
# Install OpenSSL library 
sudo apt install libssl-dev

# Compile
gcc b.c -o b -lpthread -lssl -lcrypto

# Run
./b file_path
