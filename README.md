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
