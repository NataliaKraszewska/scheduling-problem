#include <bits/stdc++.h> 
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random> 
#include <vector>


class MachineBlock{    
    int start;
    int end;
    int taskNumber;
    int operationNumber;
    std::string blockType;
    public:
    MachineBlock( int start, int end, int taskNumber, int operationNumber, std::string blockType){
        this->start = start;
        this->end = end;
        this->taskNumber = taskNumber;
        this->operationNumber = operationNumber;
        this->blockType = blockType;
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
    std::string get_blockType(){
        return blockType;
    }
};


class Task{
    int taskNumber;
    int operation1;
    int operation2;
    public:
    Task(int taskNumber, int operation1, int operation2){
    this->taskNumber = taskNumber;
    this->operation1 = operation1;
    this->operation2 = operation2;
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


void addBlock(std::vector <MachineBlock> &machineBlocks, const int start, const int end, const std::string type, const int taskNumber = 0, const int operationNumber = 0, const int maintenance = 0, const int taskLength = 0, const int maxTimeWork = 0, const float latency = 0);


void readFile(const std::string fileName, std::vector <Task> &tasks){
    std::string line;
    std::ifstream input(fileName);
    if (input.is_open()){
        while (getline (input, line)){
            int taskNumber = stoi(line.substr(line.find('t') +1,line.find(':') -1));
            std::string operations = line.substr(line.find(' ') +1,line.find(','));
            int operation1 = stoi(operations.substr(0,operations.find(',')));
            int operation2 = stoi(operations.substr(operations.find(',') +1,operations.size()));
            Task obj(taskNumber,operation1,operation2);
            tasks.push_back(obj);
        }
        input.close();
    }
}

 
int getCmax(std::vector <MachineBlock> &machineBlocks){
    int machineBlocksSize = machineBlocks.size();
    if (machineBlocksSize)
            return machineBlocks[machineBlocksSize -1].get_end();
    return 0;
}


int lastMaintenance(std::vector <MachineBlock> &machineBlocks){
    if (machineBlocks.size()){
        int lastMaintenanceEnd = 0;
        for (MachineBlock machineBlock: machineBlocks) {
            if (machineBlock.get_blockType() == "maintenance")
                lastMaintenanceEnd = machineBlock.get_end();
        }
        return lastMaintenanceEnd;
    }
    return 0;
}


int getLatency(std::vector <MachineBlock> &machineBlocks, const int maxTimeWork, const float latency){
    int endOfMachine = getCmax(machineBlocks);
    int endOfLastMaintenance = lastMaintenance(machineBlocks);
    int newMaintenanceStart = endOfLastMaintenance + maxTimeWork; 
    return ceil((newMaintenanceStart - endOfMachine) * latency); 
}


int fitTaskOnMachine(std::vector <MachineBlock> &machineBlocks, const int maintenance, const int taskLength, const int maxTimeWork, const float latency){
    int start = getCmax(machineBlocks);
    int lastMaintenanceEnd = lastMaintenance(machineBlocks);
    int additionalTask = getLatency(machineBlocks, maxTimeWork, latency);
    if (start - lastMaintenanceEnd + taskLength > maxTimeWork)
        return start + taskLength + maintenance + additionalTask;
    return  start + taskLength;
}


void addBlockWithMaintenance(std::vector <MachineBlock> &machineBlocks, int start, const int end, const std::string type, const int taskNumber, const int operationNumber, const int taskLength, const float latency, const int maintenance, const int maxTimeWork){
    int maintenanceStart = lastMaintenance(machineBlocks) + maxTimeWork;
    int time_left = end - maintenanceStart;
    addBlock(machineBlocks, start, maintenanceStart, "task_I", taskNumber, 1);
    start = maintenanceStart;
    addBlock(machineBlocks, start, start + maintenance, "maintenance");
    start = maintenanceStart+maintenance;
    addBlock(machineBlocks, start, start + latency, "latency");
    start = getCmax(machineBlocks);
    addBlock(machineBlocks, start, start + time_left, "task_II", taskNumber, 1);
}


void addBlock(std::vector <MachineBlock> &machineBlocks, const int start, const int end, const std::string type, const int taskNumber, const int operationNumber, const int maintenance, const int taskLength, const int maxTimeWork, const float latency){
    int lastMaintenanceEnd = lastMaintenance(machineBlocks);
    if (maxTimeWork!=0 && start - lastMaintenanceEnd + taskLength > maxTimeWork){
        int additionalTask = getLatency(machineBlocks,maxTimeWork,latency);
        addBlockWithMaintenance(machineBlocks, start, end, type, taskNumber, operationNumber, taskLength, additionalTask, maintenance, maxTimeWork);
    }
    else{
        MachineBlock block(start,end,taskNumber,operationNumber,type);
        machineBlocks.push_back(block);
    }
}


void addTaskToMachines(std::vector <Task> &tasks, std::vector <MachineBlock> &machine1Blocks, std::vector <MachineBlock> &machine2Blocks, const int maintenance, const int maxTimeWork, const float latency, std::vector <int> orderedTask){
    int start_1, end_1, start_2, end_2 = 0;
    for (int i=0;i<orderedTask.size();i++){    
        if(lastMaintenance(machine1Blocks) == maxTimeWork)
            addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + maintenance, "maintenance");
        if(lastMaintenance(machine2Blocks) == maxTimeWork)
            addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + maintenance, "maintenance");
        int expectedMachine1End = fitTaskOnMachine(machine1Blocks, maintenance, tasks[orderedTask[i]].get_op1(), maxTimeWork, latency);
        int noWaitBreak = 0;
        if (expectedMachine1End < getCmax(machine2Blocks)){
            noWaitBreak = getCmax(machine2Blocks) - expectedMachine1End;
            if (noWaitBreak >= maintenance){
                addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + maintenance, "maintenance");
                noWaitBreak = noWaitBreak - maintenance;
            }
            addBlock(machine1Blocks, getCmax(machine1Blocks) + noWaitBreak, getCmax(machine1Blocks) + noWaitBreak + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance,tasks[orderedTask[i]].get_op1(), maxTimeWork, latency);
            addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance, tasks[orderedTask[i]].get_op2(), maxTimeWork, latency);
        }
        else{
            noWaitBreak = expectedMachine1End - getCmax(machine2Blocks);
            if (noWaitBreak >= maintenance){
                addBlock(machine2Blocks, getCmax(machine2Blocks), getCmax(machine2Blocks) + maintenance, "maintenance");
            }
            addBlock(machine1Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op1(), "task", tasks[orderedTask[i]].get_task_number(), 1, maintenance,tasks[orderedTask[i]].get_op1(), maxTimeWork, latency);
            addBlock(machine2Blocks, getCmax(machine1Blocks), getCmax(machine1Blocks) + tasks[orderedTask[i]].get_op2(), "task", tasks[orderedTask[i]].get_task_number(), 2, maintenance,tasks[orderedTask[i]].get_op2(), maxTimeWork, latency);
        }
    }
}


std::vector <int> randomTasksOrder(const std::vector <Task> &tasks){
    std::vector <int> randomOrder;
    for (int i=0; i<tasks.size(); i++)
        randomOrder.push_back(i);
    random_shuffle(randomOrder.begin(), randomOrder.end()); 
    return randomOrder;
}


//https://stackoverflow.com/questions/33044735/c-generating-random-numbers-inside-loop/33045918
std::default_random_engine dre (std::chrono::steady_clock::now().time_since_epoch().count());
int random (int lim){
    std::uniform_int_distribution<int> uid{0, lim};
    return uid(dre);
}


std::vector <int> newTasksOrder(std::vector <int> &orderedTasks){
    int selectedTaskPosition = random(orderedTasks.size() -1);
    int selectedTask = orderedTasks[selectedTaskPosition];
    orderedTasks.erase(orderedTasks.begin()+selectedTaskPosition);
    int newSelectedTaskPosition = random(orderedTasks.size() -1);
    orderedTasks.insert(orderedTasks.begin()+newSelectedTaskPosition, selectedTask);
    return orderedTasks;
}


void writeFile(std::vector <std::vector <MachineBlock>> bestMachinesBlocks){
    std::string fileName;
    std::cout<<"Enter output file name:"<<std::endl;
    std::cin>>fileName;
    std::ofstream myfile;
    myfile.open(fileName);
    myfile<<"MACHINE 1"<<std::endl;
    for(int i=0;i<bestMachinesBlocks[0].size();i++){
        myfile<<bestMachinesBlocks[0][i].get_blockType();
        if (bestMachinesBlocks[0][i].get_taskNumber()!=0)
            myfile<<"_" <<bestMachinesBlocks[0][i].get_taskNumber();
        myfile<<" |"<<bestMachinesBlocks[0][i].get_start()<<":";
        myfile<< bestMachinesBlocks[0][i].get_end()<<"|";
        myfile<<std::endl;
    }
    myfile<<std::endl;
    myfile<<"MACHINE 2"<<std::endl;
    for(int i=0;i<bestMachinesBlocks[1].size();i++){
        myfile<<bestMachinesBlocks[1][i].get_blockType();
        if (bestMachinesBlocks[1][i].get_taskNumber() !=0)
            myfile<<"_" <<bestMachinesBlocks[1][i].get_taskNumber();
        myfile<<" |"<<bestMachinesBlocks[1][i].get_start()<<":";
        myfile<<bestMachinesBlocks[1][i].get_end()<<"|";
        myfile<<std::endl;
    }
    myfile<<std::endl;
    myfile<<"Cmax = "<<getCmax(bestMachinesBlocks[1]);
    std::cout<<"The result has been saved to a "<<fileName<<" file."<<std::endl;
    myfile.close();
}


void tabuSearch(std::vector <Task> &tasks, std::vector <MachineBlock> &machine1Blocks, std::vector <MachineBlock> &machine2Blocks, const int maintenance, const int maxTimeWork, const float latency, int neighborhood, int newInitialSolutionCount){
    std::vector < std::vector<int> > tabuList;
    int tabuListMaxSize = ceil(0.2 * tasks.size());
    std::vector <std::vector <MachineBlock>> bestCandidate;
    while(newInitialSolutionCount != 0){
        std::vector <int> initialSolution = randomTasksOrder(tasks);
        addTaskToMachines(tasks, machine1Blocks, machine2Blocks, maintenance, maxTimeWork, latency, initialSolution);
        if(bestCandidate.size() == 0){
            bestCandidate.push_back(machine1Blocks);
            bestCandidate.push_back(machine2Blocks);
        }
        else if(getCmax(machine2Blocks) < getCmax(bestCandidate[1])){
            bestCandidate.clear();
            bestCandidate.push_back(machine1Blocks);
            bestCandidate.push_back(machine2Blocks);
        }
        int improvementCount = ceil(0.3 * tasks.size());
        while(neighborhood!=0){
            std::vector <int> newCandidate = newTasksOrder(initialSolution);
            if(find(tabuList.begin(), tabuList.end(), newCandidate) == tabuList.end()){
                if(tabuList.size()==tabuListMaxSize)
                    tabuList.erase(tabuList.begin());
                tabuList.push_back(newCandidate);
                machine1Blocks.clear();
                machine2Blocks.clear();
                addTaskToMachines(tasks, machine1Blocks, machine2Blocks, maintenance, maxTimeWork, latency, newCandidate);
                if(getCmax(machine2Blocks) < getCmax(bestCandidate[1])){
                    bestCandidate.clear();
                    bestCandidate.push_back(machine1Blocks);
                    bestCandidate.push_back(machine2Blocks);
                    initialSolution = newCandidate;
                }     
                else if(getCmax(machine2Blocks) == getCmax(bestCandidate[1])){
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
    std::string inputFileName;
    std::cout<<"Enter input file path:"<<std::endl;
    std::cin>>inputFileName;
    std::cout<<"Enter maintenance length (int):"<<std::endl;
    std::cin>>maintenance;
    std::cout<<"Enter maximum machine working time without maintenance (int):"<<std::endl;
    std::cin>>maxTimeWork;
    std::cout<<"Enter latency value (float):"<<std::endl;
    std::cin>>latency;
    std::cout<<"Enter neighborhood value (int):"<<std::endl;
    std::cin>>neighborhood;
    std::cout<<"Enter how many times algorithm has to find initial solution:"<<std::endl;
    std::cin>>newInitialSolutionCount;
    std::vector <Task> tasks;
    std::vector <MachineBlock> machine1Blocks;
    std::vector <MachineBlock> machine2Blocks;
    readFile(inputFileName, tasks);
    std::cout<<"Tabu Search algorithm is working..."<<std::endl;
    tabuSearch(tasks, machine1Blocks, machine2Blocks, maintenance, maxTimeWork, latency, neighborhood, newInitialSolutionCount);
}

