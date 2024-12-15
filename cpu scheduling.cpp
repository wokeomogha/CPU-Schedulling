/* This is code developed by Oghenewoke Omogha */
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <deque>
#include <fstream>
#include <unistd.h>// for getopt
#include <stdexcept>//for runtime error
#include <getopt.h> //for long options
#define LINE_MAX 3000

using namespace std;

struct node{
    int id,
        arrive,
        burst,
        priority;

    node() : id(0), arrive(0), burst(0), priority(0) {};
};

class CPU{
    public:
       CPU() {};
       void swappingval(deque<node>::iterator num1, deque<node>::iterator num2);
       void FirstComeFirstServe(int process);
       void shortestjobfirstNP(int process);
       void Enterval(node data);
       void PriorityNonPremptive(int process);
       void RoundRobin(int quantum, int process);
       int shortestjobfirstpremptive(int process);
       void prioritypremptive(int process);
       void displayOutput(const string& outputFile);
       void readInputFile(const string& inputFile);

    private:
       deque<node> Processid;
       deque<node> Processchange;
       deque<node> make;
       deque<node> completed;
       deque<node> compared;
       deque<node> burstProcess;
};


//function for sorting
bool sortArrival(const node a, const node b);
bool sortProcess(const node a, const node b);
bool sortBurst(const node a, const node b);
bool sortPriority(const node a, const node b);

void printUsage(const char* programName) {
    cerr << "Usage: " <<programName<<"[options]\n"
         <<"Options:\n"
         <<"  -i, --input FILE   Specify input file (default: input.txt)\n"
         <<"  -o, --output FILE  Specify output file (default: output.txt)\n"
         <<"  -a, --algorithm ALG Specify scheduling algorithm:\n"
         <<"                      fcfs(First Come FIrst Serve)\n"
         <<"                      sjf(Shortest Job First)\n"
         <<"                      priority(Priority Scheduling)\n"
         <<"                      rr(Round Robin)\n"
         <<"                      sjf-preemptive\n"
         <<"                      priority-preemptive\n"
         <<"  -h, --help          Display this help message\n";
}

int main(int argc, char* argv[]) {
    //default values
    string inputFile = "input.txt";
    string outputFile = "output.txt";
    string algorithm = "";
    int quantum = 2;

    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"algorithm", required_argument, 0, 'a'},
        {"help",  no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:a:h", long_options, &option_index))== -1) {
        switch(opt) {
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'a':
                algorithm = optarg;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    CPU show;

    //Read input file
    try{
        show.readInputFile(inputFile);
    } catch (const exception& e) {
        cerr <<"Error reading input file: " <<e.what() <<endl;
        return 1;
    }

    //Determine the  number of processes
    int process = show.getProcessCount();

    //Execute selected algorithm
    if (algorithm == "fcfs"){
        show.FirstComeFirstServe(process);
    } else if (algorithm == "sjf"){
        show.shortestjobfirstNP(process);
    } else if (algorithm == "priority") {
        show.PriorityNonPremptive(process);
    } else if (algorithm == "rr") {
        show.RoundRobin(quantum, process);
    } else if (algorithm == "sjf-preemptive") {
        show.shortestjobfirstpremptive(process);
    } else if (algorithm == "priority-preemptive") {
        show.prioritypremptive(process);
    } else {
        cerr << "No or invalid algorithm specified. Use -h for help\n";
        return 0;
    }

    show.displayOutput(outputFile);

    return 1;
}

void CPU::readInputFile(const string& inputFile) {
    FILE *fp;
    char line[LINE_MAX];
    unsigned int num[3];
    int process = 0;

    if ((fp = fopen("input.txt", "r"))== NULL) {
        throw runtime_error("Could not open file");
    }

    while (fgets(line, LINE_MAX, fp)!= NULL) {
        if (sscanf(line, "%d:%d:%d\n", &num[0], num[1], num[2]) == 3) {
            node data;
            data.id = ++process;
            data.burst = num[0];
            data.arrive = num[1];
            data.priority = num[2];
            Enterval(data);
        }
    }
    fclose(fp);
}

void CPU::displayOutput(const string& outputFile){
    if(remove("output.txt")!=0)
        cerr << "Could not remove file\n";
    else
        puts("Output file cleared automatically");
    int choice;
       while(1){
           
           cout<<"..........................................."<<endl;
           cout<<"CPU SCHEDULER SIMULATOR"<<endl;
           cout<<"1. Scheduling Method"<<endl;
           cout<<"2. Preemptive Method"<<endl;
           cout<<"3. Non Preemptive Method"<<endl;
           cout<<"4. Show Result"<<endl;
           cout<<"5. End Program"<<endl;
           cout<<"Option"<<endl;
                  int quantum = 2;
            cin>>choice;
            switch(choice){
                case 1:
                       cout<<"1. First Come First Serve Scheduling:"<<endl;
                       cout<<"2. Shortest Job First Scheduling:"<<endl;
                       cout<<"3. Priority Scheduling:"<<endl;
                       cout<<"4. Round Robin Scheduling:"<<endl;
                       cout<<"Option"<<endl;
                         int option;
                         cin>>option;
                        if(option==1){
                               cout<<"output for first come first serve is written in file"<<endl;
                               show.FirstComeFirstServe(process);
                        }
                        else if(option==2){
                            cout<<"output for shortest job first is written in file"<<endl;
                            show.shortestjobfirstNP(process);
                        }
                        else if(option==3){
                            cout<<"output for priority is written in file"<<endl;
                            show.PriorityNonPremptive(process);
                        }
                        else if(option==4){
                            cout<<"output for round robin is written in file"<<endl;
                            show.RoundRobin(quantum, process);
                        }

                break;
            }
       }
}