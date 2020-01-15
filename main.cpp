#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <bits/stdc++.h> 
#include <random> 
#include <chrono>   
#include <algorithm>


using namespace std;


class MachineBlock{    
    int start;
    int end;
    int taskNumber;
    int operationNumber;
    string blockType;
    public:
    void set_start(int st){
        start = st;
    }
    void set_end(int en){
        end = en;
    }
    void set_taskNumber(int tN){
        taskNumber = tN;
    }
    void set_operationNumber(int oN){
        operationNumber = oN;
    }
    void set_blockType(string bT){
        blockType = bT;
    }
    int get_start(){
        return start;
    }
    int get_end(){
        return end;
    }
    int get_taskNumber(){
        return taskNumber;
    }
    int get_operationNumber(){
        return operationNumber;
    }
    string get_blockType(){
        return blockType;
    }
};


class Task{
    int taskNumber;
    int operation1;
    int operation2;
    public:
    void set_task_number(int tN){
        taskNumber = tN;
    }
    void set_op1(int op1){  
        operation1 = op1;
    }
    void set_op2(int op2){
        operation2 = op2;
    }
    int get_task_number(){
        return taskNumber;
    }
    int get_op1(){
        return operation1;
    }
    int get_op2(){
        return operation2;
    }
};


void addBlock(vector <MachineBlock> &machine, const int start, const int end, const string type, const int taskNumber = 0, const int operationNumber = 0, const int maintenance = 0, const int taskLength = 0, const int maxTimeWork = 0, const float latency = 0);


void readFile(const string fileName, vector <Task> &tasks){
    string line;
    ifstream input (fileName);
    if (input.is_open()){
        while (getline (input, line)){
            int taskNumber = stoi(line.substr(line.find('t') +1,line.find(':') -1));
            string operations = line.substr(line.find(' ') +1,line.find(','));
            int operation1 = stoi(operations.substr(0,operations.find(',')));
            int operation2 = stoi(operations.substr(operations.find(',') +1,operations.size()));
            Task obj;
            obj.set_task_number(taskNumber);
            obj.set_op1(operation1);
            obj.set_op2(operation2);
            tasks.push_back(obj);
        }
        input.close();
    }
}

 
int getCmax(vector <MachineBlock> &machine){
    if (machine.size() != 0)
            return machine[machine.size() -1].get_end();
    return 0;
}


int lastMaintenance(vector <MachineBlock> &machine){
    if (machine.size() == 0)
        return 0;
    int lastMaintenanceEnd = 0;
    for(int i=0;i<machine.size();i++){
        if (machine[i].get_blockType() == "maintenance")
            lastMaintenanceEnd = machine[i].get_end();
    }
    return lastMaintenanceEnd;
}


int getLatency(vector <MachineBlock> &machine, const int maxTimeWork, const float latency){
    int endOfMachine = getCmax(machine);
    int endOfLastMaintenance = lastMaintenance(machine);
    int newMaintenanceStart = endOfLastMaintenance + maxTimeWork; 
    return ceil((newMaintenanceStart - endOfMachine) * latency); 
}


int fitTaskOnMachine(vector <MachineBlock> &machine, const int maintenance, const int taskLength, const int maxTimeWork, const float latency){
    int start = getCmax(machine);
    int lastMaintenanceEnd = lastMaintenance(machine);
    int additionalTask = getLatency(machine, maxTimeWork, latency);
    if (start - lastMaintenanceEnd + taskLength > maxTimeWork)
        return start + taskLength + maintenance + additionalTask;
    return  start + taskLength;
}


void addBlockWithMaintenance(vector <MachineBlock> &machine, int start, const int end, const string type, const int taskNumber, const int operationNumber, const int taskLength, const float latency, const int maintenance, const int maxTimeWork){
    int maintenanceStart = lastMaintenance(machine) + maxTimeWork;
    int time_left = end - maintenanceStart;
    addBlock(machine, start, maintenanceStart, "task_I", taskNumber, 1);
    start = maintenanceStart;
    addBlock(machine, start, start + maintenance, "maintenance");
    start = maintenanceStart+maintenance;
    addBlock(machine, start, start + latency, "latency");
    start = getCmax(machine);
    addBlock(machine, start, start + time_left, "task_II", taskNumber, 1);
}


void addBlock(vector <MachineBlock> &machine, const int start, const int end, const string type, const int taskNumber, const int operationNumber, const int maintenance, const int taskLength, const int maxTimeWork, const float latency){
    int lastMaintenanceEnd = lastMaintenance(machine);
    if (maxTimeWork!=0 && start - lastMaintenanceEnd + taskLength > maxTimeWork){
        int additionalTask = getLatency(machine,maxTimeWork,latency);
        addBlockWithMaintenance(machine, start, end, type, taskNumber, operationNumber, taskLength, additionalTask, maintenance, maxTimeWork);
    }
    else{
        MachineBlock block;
        block.set_start(start);
        block.set_end(end);
        block.set_blockType(type);
        block.set_operationNumber(operationNumber);
        block.set_taskNumber(taskNumber);
        machine.push_back(block);
    }
}


void AddTaskToMachines(vector <Task> &tasks, vector <MachineBlock> &machine1, vector <MachineBlock> &machine2, const int maintenance, const int maxTimeWork, const float latency, vector <int> orderedTask){
    int start_1, end_1, start_2, end_2 = 0;
    for (int i=0;i<orderedTask.size();i++){    
        if(lastMaintenance(machine1) == maxTimeWork)
            addBlock(machine1, getCmax(machine1), getCmax(machine1) + maintenance, "maintenance");
        if(lastMaintenance(machine2) == maxTimeWork)
            addBlock(machine2, getCmax(machine2), getCmax(machine2) + maintenance, "maintenance");
        int expectedMachine1End = fitTaskOnMachine(machine1, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork, latency);
        int noWaitBreak = 0;
        if (expectedMachine1End < getCmax(machine2)){
            noWaitBreak = getCmax(machine2) - expectedMachine1End;
            if (noWaitBreak >= maintenance){
                addBlock(machine1, getCmax(machine1), getCmax(machine1) + maintenance, "maintenance");
                noWaitBreak = noWaitBreak - maintenance;
            }
            addBlock(machine1, getCmax(machine1) + noWaitBreak, getCmax(machine1) + noWaitBreak + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance,tasks[orderedTask[i]].get_op1(), maxTimeWork, latency);
            addBlock(machine2, getCmax(machine1), getCmax(machine1) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork, latency);
        }
        else{
            noWaitBreak = expectedMachine1End - getCmax(machine2);
            if (noWaitBreak >= maintenance){
                addBlock(machine2, getCmax(machine2), getCmax(machine2) + maintenance, "maintenance");
            }
            addBlock(machine1, getCmax(machine1), getCmax(machine1) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance,tasks[orderedTask[i]].get_op1(), maxTimeWork, latency);
            addBlock(machine2, getCmax(machine1), getCmax(machine1) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance,tasks[orderedTask[i]].get_op2(), maxTimeWork, latency);
        }
    }
}


vector <int> randomTasksOrder(const vector <Task> &tasks){
    vector <int> randomOrder;
    for (int i=0; i<tasks.size(); i++){
        randomOrder.push_back(i);
    }
    random_shuffle(randomOrder.begin(), randomOrder.end()); 
    return randomOrder;
}


//https://stackoverflow.com/questions/33044735/c-generating-random-numbers-inside-loop/33045918
default_random_engine dre (chrono::steady_clock::now().time_since_epoch().count());
int random (int lim){
    uniform_int_distribution<int> uid {0,lim};
    return uid(dre);
}


vector <int> newTasksOrder(vector <int> &tasksOrder){
    int selectedTaskPosition = random(tasksOrder.size() -1);
    int selectedTask = tasksOrder[selectedTaskPosition];
    tasksOrder.erase(tasksOrder.begin()+selectedTaskPosition);
    int newSelectedTaskPosition = random(tasksOrder.size() -1);
    tasksOrder.insert(tasksOrder.begin()+newSelectedTaskPosition, selectedTask);
    return tasksOrder;
}


void writeFile(vector <vector <MachineBlock>> bestCandidate){
    string fileName;
    cout<<"Enter output file name:\n";
    cin>>fileName;
    ofstream myfile;
    myfile.open(fileName);
    myfile<<"MACHINE 1 \n";
    for(int i=0;i<bestCandidate[0].size();i++){
        myfile<<bestCandidate[0][i].get_blockType();
        if (bestCandidate[0][i].get_taskNumber()!=0)
            myfile<<"_" <<bestCandidate[0][i].get_taskNumber();
        myfile<<" |"<<bestCandidate[0][i].get_start()<<":";
        myfile<< bestCandidate[0][i].get_end()<<"|";
        myfile<<endl;
    }
    myfile<<endl;
    myfile<<"MACHINE 2 \n";
    for(int i=0;i<bestCandidate[1].size();i++){
        myfile<<bestCandidate[1][i].get_blockType();
        if (bestCandidate[1][i].get_taskNumber() !=0)
            myfile<<"_" << bestCandidate[1][i].get_taskNumber();
        myfile<<" |"<<bestCandidate[1][i].get_start()<<":";
        myfile<<bestCandidate[1][i].get_end()<<"|";
        myfile<<endl;
    }
    myfile<<endl;
    myfile<<"Cmax = "<<getCmax(bestCandidate[1]);
    cout<<"The result has been saved to a "<<fileName<<" file."<<endl;
    myfile.close();
}


void TabuSearch(vector <Task> &tasks, vector <MachineBlock> &machine1, vector <MachineBlock> &machine2, const int maintenance, const int maxTimeWork, const float latency, int neighborhood, int newInitialSolutionCount){
    vector < vector<int> > tabuList;
    int tabuListMaxSize = ceil(0.2 * tasks.size());
    vector <vector <MachineBlock>> bestCandidate;
    while(newInitialSolutionCount!=0){
        vector <int> initialSolution = randomTasksOrder(tasks);
        AddTaskToMachines(tasks, machine1, machine2, maintenance, maxTimeWork, latency, initialSolution);
        if(bestCandidate.size()==0){
            bestCandidate.push_back(machine1);
            bestCandidate.push_back(machine2);
        }
        else if(getCmax(machine2) < getCmax(bestCandidate[1])){
            bestCandidate.clear();
            bestCandidate.push_back(machine1);
            bestCandidate.push_back(machine2);
        }
        int improvementCount = ceil(0.3 * tasks.size());
        while(neighborhood!=0){
            vector <int> newCandidate = newTasksOrder(initialSolution);
            if(find(tabuList.begin(), tabuList.end(), newCandidate) == tabuList.end()){
                if(tabuList.size()==tabuListMaxSize)
                    tabuList.erase(tabuList.begin());
                tabuList.push_back(newCandidate);
                machine1.clear();
                machine2.clear();
                AddTaskToMachines(tasks, machine1, machine2, maintenance, maxTimeWork, latency, newCandidate);
                if(getCmax(machine2) < getCmax(bestCandidate[1])){
                    bestCandidate.clear();
                    bestCandidate.push_back(machine1);
                    bestCandidate.push_back(machine2);
                    initialSolution = newCandidate;
                }     
                else if(getCmax(machine2) == getCmax(bestCandidate[1])){
                    improvementCount -= 1;
                    if (improvementCount == 0)
                        neighborhood = 0;
                }            
                neighborhood -= 1;
                }
        }
        newInitialSolutionCount -= 1;    
    }
    writeFile(bestCandidate);
}


int main(){
    int maintenance;
    int maxTimeWork;
    float latency;
    int neighborhood;
    int newInitialSolutionCount;
    string inputFileName;
    cout<<"Enter input file path:\n";
    cin>>inputFileName;
    cout<<"Enter maintenance length (int):\n";
    cin>>maintenance;
    cout<<"Enter maximum machine working time without maintenance (int):\n";
    cin>>maxTimeWork;
    cout<<"Enter latency value (float):\n";
    cin>>latency;
    cout<<"Enter neighborhood value (int):\n";
    cin>>neighborhood;
    cout<<"Enter how many times algorithm has to find initial solution:\n";
    cin>>newInitialSolutionCount;
    vector <Task> tasks;
    vector <MachineBlock> machine1;
    vector <MachineBlock> machine2;
    readFile(inputFileName, tasks);
    cout<<"Tabu Search algorithm is working...\n"<<endl;
    TabuSearch(tasks, machine1, machine2, maintenance, maxTimeWork, latency, neighborhood, newInitialSolutionCount);
}

