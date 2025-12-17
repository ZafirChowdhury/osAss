## Prerequisites & Setup
Run the following commands to install dependencies:

```bash
# Update package list
sudo apt update

# Install GCC, Make, and standard build tools
sudo apt install build-essential

# Install OpenSSL library (Required for Part B)
sudo apt install libssl-dev

# Compile	
gcc source.c -o output_name

# Run
./output_name
