import sys

class Process:
    def __init__(self, pid, arrival_time, burst_time, queue_id):
        self.pid = pid
        self.arrival_time = arrival_time
        self.burst_time = burst_time
        self.queue_id = queue_id  # 0=High, 1=Medium, 2=Low
        
        # Tracking states
        self.remaining_time = burst_time
        self.start_time = None  # When it first gets CPU
        self.completion_time = 0
        self.waiting_time = 0
        self.turnaround_time = 0
        self.response_time = 0

class MLQScheduler:
    def __init__(self):
        # Time tracking
        self.current_time = 0
        self.busy_time = 0
        
        # Queues: List of lists. 
        # Index 0 is highest priority.
        self.queues = [[], [], []] 
        
        # Scheduling rules per queue
        # Format: (Algorithm, Quantum). None for FCFS.
        self.queue_meta = [
            {"algo": "RR", "quantum": 2},  # Queue 0
            {"algo": "RR", "quantum": 4},  # Queue 1
            {"algo": "FCFS", "quantum": None} # Queue 2
        ]
        
        self.incoming_processes = []
        self.completed_processes = []
        self.gantt_chart = [] # To visualize execution
        
        # To track Round Robin state
        self.current_process = None
        self.current_q_id = -1
        self.time_slice_counter = 0

    def add_process(self, pid, arrival, burst, queue_id):
        p = Process(pid, arrival, burst, queue_id)
        self.incoming_processes.append(p)
        # Sort by arrival time to simulate timeline correctly
        self.incoming_processes.sort(key=lambda x: x.arrival_time, reverse=True)

    def get_highest_priority_non_empty_queue(self):
        # Check queues 0, 1, 2 in order
        for i in range(len(self.queues)):
            if len(self.queues[i]) > 0:
                return i
        return -1

    def run(self):
        print("--- Simulation Started ---")
        
        # Loop until all processes are completed
        while True:
            # 1. Check for new arrivals
            # We iterate backwards to remove items safely while looping
            for i in range(len(self.incoming_processes) - 1, -1, -1):
                p = self.incoming_processes[i]
                if p.arrival_time <= self.current_time:
                    self.queues[p.queue_id].append(p)
                    self.incoming_processes.pop(i)
                    print(f"[Time {self.current_time}] Process {p.pid} arrived -> Queue {p.queue_id}")

            # 2. Check termination condition
            if not self.incoming_processes and \
               sum(len(q) for q in self.queues) == 0 and \
               self.current_process is None:
                break

            # 3. Determine which queue should run (Preemption Logic)
            active_q_index = self.get_highest_priority_non_empty_queue()

            # Logic: If a higher priority queue has processes, we must preempt
            # unless the current process is already from that high priority queue
            if self.current_process and active_q_index != -1:
                if active_q_index < self.current_q_id:
                    # PREEMPTION HAPPENS HERE
                    print(f"[Time {self.current_time}] Preempting Process {self.current_process.pid} (Q{self.current_q_id}) for Q{active_q_index}")
                    # Put current back to head of its own queue (RR logic)
                    self.queues[self.current_q_id].append(self.current_process)
                    self.current_process = None
                    self.time_slice_counter = 0
            
            # 4. Load a process if CPU is idle
            if self.current_process is None and active_q_index != -1:
                # Get first process from the highest priority queue
                self.current_process = self.queues[active_q_index].pop(0)
                self.current_q_id = active_q_index
                self.time_slice_counter = 0
                
                # Record Response Time (first time getting CPU)
                if self.current_process.start_time is None:
                    self.current_process.start_time = self.current_time
                    self.current_process.response_time = self.current_time - self.current_process.arrival_time

            # 5. Execute Process (1 tick)
            if self.current_process:
                self.current_process.remaining_time -= 1
                self.busy_time += 1
                self.time_slice_counter += 1
                self.gantt_chart.append(self.current_process.pid)

                # Check if finished
                if self.current_process.remaining_time == 0:
                    self.current_process.completion_time = self.current_time + 1
                    self.calculate_metrics(self.current_process)
                    self.completed_processes.append(self.current_process)
                    print(f"[Time {self.current_time+1}] Process {self.current_process.pid} Finished")
                    self.current_process = None
                    self.time_slice_counter = 0
                
                # Check Round Robin Time Quantum
                else:
                    algo_meta = self.queue_meta[self.current_q_id]
                    if algo_meta["algo"] == "RR":
                        if self.time_slice_counter >= algo_meta["quantum"]:
                            # Time slice expired, move to back of queue
                            print(f"[Time {self.current_time+1}] Process {self.current_process.pid} Time Quantum Expired -> Re-queued")
                            self.queues[self.current_q_id].append(self.current_process)
                            self.current_process = None
                            self.time_slice_counter = 0
            else:
                self.gantt_chart.append("IDLE")

            # 6. Advance Time
            self.current_time += 1

    def calculate_metrics(self, p):
        p.turnaround_time = p.completion_time - p.arrival_time
        p.waiting_time = p.turnaround_time - p.burst_time

    def print_metrics(self):
        print("\n" + "="*65)
        print(f"{'PID':<6} {'Q':<4} {'Arr':<6} {'Burst':<6} {'Finish':<8} {'TurnAr':<8} {'Wait':<6} {'Resp':<6}")
        print("-" * 65)
        
        total_tat = 0
        total_wt = 0
        
        for p in sorted(self.completed_processes, key=lambda x: x.pid):
            total_tat += p.turnaround_time
            total_wt += p.waiting_time
            print(f"{p.pid:<6} {p.queue_id:<4} {p.arrival_time:<6} {p.burst_time:<6} "
                  f"{p.completion_time:<8} {p.turnaround_time:<8} {p.waiting_time:<6} {p.response_time:<6}")
        
        print("-" * 65)
        n = len(self.completed_processes)
        print(f"Average Turnaround Time: {total_tat/n:.2f}")
        print(f"Average Waiting Time:    {total_wt/n:.2f}")
        print(f"CPU Utilization:         {(self.busy_time / self.current_time) * 100:.2f}%")
        print(f"Throughput:              {n / self.current_time:.4f} processes/unit time")
        print("="*65)
        
        # Simplified Gantt Chart visualization
        print("\nExecution Timeline (Gantt Chart):")
        print(str(self.gantt_chart))

# --- Main Execution ---
if __name__ == "__main__":
    scheduler = MLQScheduler()

    # Adding Processes (ID, Arrival, Burst, Queue_ID)
    # Queue 0 (High Priority, RR, Q=2)
    scheduler.add_process("P1", 0, 4, 0)
    
    # Queue 1 (Medium Priority, RR, Q=4)
    scheduler.add_process("P2", 1, 3, 1)
    
    # Queue 2 (Low Priority, FCFS)
    scheduler.add_process("P3", 2, 5, 2) 
    
    # Delayed High Priority Arrival to test Preemption
    scheduler.add_process("P4", 5, 2, 0) 

    scheduler.run()
    scheduler.print_metrics()
    