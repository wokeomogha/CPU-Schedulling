// cpe351.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <cstdlib>

struct Process {
    int id;  // Process ID for tracking original order
    int burst_time;
    int arrival_time;
    int priority;
    int waiting_time;
    int remaining_time;
    int completion_time;
    Process* next;
    
    Process(int pid, int bt, int at, int pr) : 
        id(pid), burst_time(bt), arrival_time(at), priority(pr),
        waiting_time(0), remaining_time(bt), completion_time(0),
        next(nullptr) {}
};

class ProcessList {
private:
    Process* head;
    int size;

public:
    ProcessList() : head(nullptr), size(0) {}
    
    void insert(int id, int burst_time, int arrival_time, int priority) {
        Process* newProcess = new Process(id, burst_time, arrival_time, priority);
        if (!head) {
            head = newProcess;
        } else {
            Process* current = head;
            while (current->next) {
                current = current->next;
            }
            current->next = newProcess;
        }
        size++;
    }
    
    Process* getHead() { return head; }
    int getSize() { return size; }
    
    void resetProcesses() {
        Process* current = head;
        while (current) {
            current->waiting_time = 0;
            current->remaining_time = current->burst_time;
            current->completion_time = 0;
            current = current->next;
        }
    }

    ProcessList* clone() {
        ProcessList* newList = new ProcessList();
        Process* current = head;
        while (current) {
            newList->insert(current->id, current->burst_time, 
                          current->arrival_time, current->priority);
            current = current->next;
        }
        return newList;
    }

    ~ProcessList() {
        while (head) {
            Process* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

class Scheduler {
private:
    ProcessList* processes;
    int quantum;
    int process_count;

    void calculateFCFS() {
        Process* current = processes->getHead();
        int current_time = 0;
        
        // Sort processes by arrival time (bubble sort)
        for (int i = 0; i < process_count - 1; i++) {
            Process* current = processes->getHead();
            while (current->next) {
                if (current->arrival_time > current->next->arrival_time) {
                    // Swap process data
                    std::swap(current->id, current->next->id);
                    std::swap(current->burst_time, current->next->burst_time);
                    std::swap(current->arrival_time, current->next->arrival_time);
                    std::swap(current->priority, current->next->priority);
                }
                current = current->next;
            }
        }
        
        current = processes->getHead();
        while (current) {
            if (current_time < current->arrival_time) {
                current_time = current->arrival_time;
            }
            
            current->waiting_time = current_time - current->arrival_time;
            current_time += current->burst_time;
            current->completion_time = current_time;
            current = current->next;
        }
    }

    void calculateSJFNonPreemptive() {
        int current_time = 0;
        int completed = 0;
        
        while (completed != process_count) {
            Process* shortest = nullptr;
            int min_burst = INT_MAX;
            
            Process* current = processes->getHead();
            while (current) {
                if (current->arrival_time <= current_time && 
                    current->remaining_time > 0 && 
                    current->remaining_time < min_burst) {
                    min_burst = current->remaining_time;
                    shortest = current;
                }
                current = current->next;
            }
            
            if (!shortest) {
                current_time++;
                continue;
            }
            
            shortest->waiting_time = current_time - shortest->arrival_time;
            current_time += shortest->burst_time;
            shortest->remaining_time = 0;
            shortest->completion_time = current_time;
            completed++;
        }
    }

    void calculateSJFPreemptive() {
        int current_time = 0;
        int completed = 0;
        
        while (completed != process_count) {
            Process* shortest = nullptr;
            int min_remaining = INT_MAX;
            
            Process* current = processes->getHead();
            while (current) {
                if (current->arrival_time <= current_time && 
                    current->remaining_time > 0 && 
                    current->remaining_time < min_remaining) {
                    min_remaining = current->remaining_time;
                    shortest = current;
                }
                current = current->next;
            }
            
            if (!shortest) {
                current_time++;
                continue;
            }
            
            shortest->remaining_time--;
            current_time++;
            
            if (shortest->remaining_time == 0) {
                completed++;
                shortest->completion_time = current_time;
                shortest->waiting_time = shortest->completion_time - 
                                      shortest->arrival_time - 
                                      shortest->burst_time;
            }
        }
    }

    void calculatePriorityNonPreemptive() {
        int current_time = 0;
        int completed = 0;
        
        while (completed != process_count) {
            Process* highest_priority = nullptr;
            int max_priority = INT_MIN;
            
            Process* current = processes->getHead();
            while (current) {
                if (current->arrival_time <= current_time && 
                    current->remaining_time > 0 && 
                    current->priority > max_priority) {
                    max_priority = current->priority;
                    highest_priority = current;
                }
                current = current->next;
            }
            
            if (!highest_priority) {
                current_time++;
                continue;
            }
            
            highest_priority->waiting_time = current_time - highest_priority->arrival_time;
            current_time += highest_priority->burst_time;
            highest_priority->remaining_time = 0;
            highest_priority->completion_time = current_time;
            completed++;
        }
    }

    void calculatePriorityPreemptive() {
        int current_time = 0;
        int completed = 0;
        
        while (completed != process_count) {
            Process* highest_priority = nullptr;
            int max_priority = INT_MIN;
            
            Process* current = processes->getHead();
            while (current) {
                if (current->arrival_time <= current_time && 
                    current->remaining_time > 0 && 
                    current->priority > max_priority) {
                    max_priority = current->priority;
                    highest_priority = current;
                }
                current = current->next;
            }
            
            if (!highest_priority) {
                current_time++;
                continue;
            }
            
            highest_priority->remaining_time--;
            current_time++;
            
            if (highest_priority->remaining_time == 0) {
                completed++;
                highest_priority->completion_time = current_time;
                highest_priority->waiting_time = highest_priority->completion_time - 
                                              highest_priority->arrival_time - 
                                              highest_priority->burst_time;
            }
        }
    }

    void calculateRoundRobin() {
        int current_time = 0;
        int completed = 0;
        
        while (completed != process_count) {
            bool processed = false;
            Process* current = processes->getHead();
            
            while (current) {
                if (current->arrival_time <= current_time && current->remaining_time > 0) {
                    processed = true;
                    
                    if (current->remaining_time > quantum) {
                        current_time += quantum;
                        current->remaining_time -= quantum;
                    } else {
                        current_time += current->remaining_time;
                        current->completion_time = current_time;
                        current->waiting_time = current->completion_time - 
                                             current->arrival_time - 
                                             current->burst_time;
                        current->remaining_time = 0;
                        completed++;
                    }
                }
                current = current->next;
            }
            
            if (!processed) current_time++;
        }
    }

public:
    Scheduler(int q = 2) : processes(nullptr), quantum(q), process_count(0) {}

    void loadProcesses(const std::string& input.txt) {
        std::ifstream file(input.txt);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open input file");
        }

        processes = new ProcessList();
        std::string line;
        int id = 0;
        
        while (std::getline(file, line)) {
            int burst_time, arrival_time, priority;
            if (sscanf(line.c_str(), "%d:%d:%d", &burst_time, &arrival_time, &priority) == 3) {
                processes->insert(id++, burst_time, arrival_time, priority);
                process_count++;
            }
        }
        file.close();
    }

    void runAllAlgorithms(const std::string& output.txt) {
        std::ofstream outFile(output.txt);
        if (!outFile.is_open()) {
            throw std::runtime_error("Could not open output file");
        }

        // Run each algorithm
        ProcessList* original = processes->clone();
        
        // FCFS
        calculateFCFS();
        writeResults(1, outFile);
        processes = original->clone();
        processes->resetProcesses();

        // SJF Non-preemptive
        calculateSJFNonPreemptive();
        writeResults(2, outFile);
        processes = original->clone();
        processes->resetProcesses();

        // SJF Preemptive
        calculateSJFPreemptive();
        writeResults(3, outFile);
        processes = original->clone();
        processes->resetProcesses();

        // Priority Non-preemptive
        calculatePriorityNonPreemptive();
        writeResults(4, outFile);
        processes = original->clone();
        processes->resetProcesses();

        // Priority Preemptive
        calculatePriorityPreemptive();
        writeResults(5, outFile);
        processes = original->clone();
        processes->resetProcesses();

        // Round Robin
        calculateRoundRobin();
        writeResults(6, outFile);

        outFile.close();
        delete original;
    }

private:
    void writeResults(int algorithm_id, std::ofstream& outFile) {
        // Prepare output string
        std::string output = std::to_string(algorithm_id);
        float total_waiting_time = 0;

        // Create a temporary array for sorting by original process ID
        struct TempResult {
            int id;
            int waiting_time;
        };
        TempResult* results = new TempResult[process_count];
        int idx = 0;

        // Store results
        Process* current = processes->getHead();
        while (current) {
            results[idx].id = current->id;
            results[idx].waiting_time = current->waiting_time;
            total_waiting_time += current->waiting_time;
            current = current->next;
            idx++;
        }

        // Sort by original process ID
        for (int i = 0; i < process_count - 1; i++) {
            for (int j = 0; j < process_count - i - 1; j++) {
                if (results[j].id > results[j + 1].id) {
                    std::swap(results[j], results[j + 1]);
                }
            }
        }

        // Build output string
        for (int i = 0; i < process_count; i++) {
            output += ":" + std::to_string(results[i].waiting_time);
        }

        float avg_waiting_time = total_waiting_time / process_count;
        output += ":" + std::to_string(avg_waiting_time);

        // Write to both file and screen
        outFile << output << std::endl;
        std::cout << output << std::endl;

        delete[] results;
    }
};

void print_usage() {
    std::cerr << "Usage: ./cpe351 -t quantum -f input.txt -o output.txt\n"
              << "Options:\n"
              << "  -t  Time quantum for Round Robin scheduling\n"
              << "  -f  Input file name\n"
              << "  -o  Output file name\n";
}

int main(int argc, char* argv[]) {
    int quantum = 2;  // default value
    std::string input.txt, output.txt;
    int opt;
    bool has_input = false, has_output = false;

    // Parse command line arguments using getopt
    while ((opt = getopt(argc, argv, "t:f:o:")) != -1) {
        switch (opt) {
            case 't':
                quantum = std::atoi(optarg);
                if (quantum <= 0) {
                    std::cerr << "Error: Time quantum must be positive\n";
                    return 1;
                }
                break;
            case 'f':
                input_file = optarg;
                has_input = true;
                break;
            case 'o':
                output_file = optarg;
                has_output = true;
                break;
            default:
                print_usage();
                return 1;
        }
    }

    // Check if required arguments are provided
    if (!has_input || !has_output) {
        std::cerr << "Error: Input and output files are required\n";
        print_usage();
        return 1;
    }
    
    try {
        Scheduler scheduler(quantum);
        scheduler.loadProcesses(input.txt);
        scheduler.runAllAlgorithms(output.txt);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
