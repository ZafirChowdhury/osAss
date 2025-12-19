import threading
import time
import random

# --- Constants for States ---
THINKING = 0
HUNGRY = 1
EATING = 2
N = 5  # Number of philosophers

class DiningServer:
    """
    The Monitor Class.
    It manages the shared state of the simulation.
    """
    def __init__(self):
        self.lock = threading.Lock() # Global lock for the monitor
        self.state = [THINKING] * N # List of states
        # One Condition Variable per philosopher to make them wait if hungry
        self.cvs = [threading.Condition(self.lock) for _ in range(N)]

    def _left(self, i):
        return (i - 1) % N

    def _right(self, i):
        return (i + 1) % N

    def test(self, i):
        """
        Checks if philosopher 'i' can eat.
        They can eat only if:
        1. They are HUNGRY
        2. Left neighbor is NOT Eating
        3. Right neighbor is NOT Eating
        """
        if (self.state[i] == HUNGRY and 
            self.state[self._left(i)] != EATING and 
            self.state[self._right(i)] != EATING):
            
            # Update state
            self.state[i] = EATING
            
            # Wake up the philosopher (if they were waiting)
            self.cvs[i].notify()

    def takeForks(self, i):
        with self.lock:
            # 1. Announce hunger
            self.state[i] = HUNGRY
            print(f"Philosopher {i} is HUNGRY")
            
            # 2. Try to get forks
            self.test(i)
            
            # 3. If unsuccessful, wait here until a neighbor signals us
            while self.state[i] != EATING:
                self.cvs[i].wait()
            
            print(f"Philosopher {i} has acquired forks and is STARTED EATING")

    def returnForks(self, i):
        with self.lock:
            # 1. Done eating
            print(f"Philosopher {i} put down forks and is THINKING")
            self.state[i] = THINKING
            
            # 2. Check if neighbors can now eat
            # Since I put down my forks, my neighbors might be able to start
            self.test(self._left(i))
            self.test(self._right(i))

class Philosopher(threading.Thread):
    def __init__(self, id, server):
        super().__init__()
        self.id = id
        self.server = server

    def run(self):
        # Loop a few times for the simulation
        for _ in range(3): 
            self.think()
            self.server.takeForks(self.id)
            self.eat()
            self.server.returnForks(self.id)
        
        print(f"--- Philosopher {self.id} finished dinner ---")

    def think(self):
        time.sleep(random.uniform(1, 3)) # Think for random time

    def eat(self):
        time.sleep(random.uniform(1, 2)) # Eat for random time

# --- Main Simulation ---
if __name__ == "__main__":
    server = DiningServer()
    philosophers = []

    print("--- Dinner Started ---")

    # Create Threads
    for i in range(N):
        p = Philosopher(i, server)
        philosophers.append(p)
        p.start()

    # Wait for completion
    for p in philosophers:
        p.join()

    print("--- Dinner Finished ---")
