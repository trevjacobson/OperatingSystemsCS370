#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

#define MAX_PROCESS 80 

class Process{      //Process class deals with the values of various variables the process will take on over the course of execution on a process
public:
  Process(){
    pid = startTime = endTime = initPriority = priority = timeSlice = totalCpuBurst = totalCpuTime = totalIoTime = 0;
  }
  int getPid(){return pid;}                           //process ID
  int getStartTime(){return startTime;}               //start time for a process
  int getEndTime(){return endTime;}                   //ending time for a process
  int getInitPriority(){return initPriority;}         //holds the initial priority for a process
  int getPriority(){return priority;}                 //return process priority level
  int getTimeSlice(){return timeSlice;}               //return time slice duration
  int getNumCpuBurst(){return numCpuBurst[0];}        //return time remaining on current CPU burst
  int getNumIoBurst(){return numIoBurst[0];}          //return time remaining on current IO burst
  int getTotalCpuBurst(){return totalCpuBurst;}       //number of Cpu bursts
  int getTotalCpuTime(){return totalCpuTime;}         //total time spent in CPU
  int getTotalIoTime(){return totalIoTime;}           //total time spent in IO
  int getCpuBurstSize(){return numCpuBurst.size();}   //return the size of the vector holding cpu bursts
  int getIoBurstSize(){return numIoBurst.size();}     //return the size of the vector holding io bursts
  void setTotalCpuBurst(int t){totalCpuBurst = t;}    //total number of cpu bursts for a process
  void setTotalCpuTime(){totalCpuTime++;}             //total cpu time process has done so far
  void setTotalIoTime(){totalIoTime++;}               //total io time process has done so far
  void setPid(int proId){pid = proId;}                //set the process' ID
  void setEndTime(int e){endTime = e;}                //set ending time of a process
  void setInitPriority(int nice){                     //set the initial priority of a process
    initPriority = (int)(((nice + 20)/39.0)*30 + 0.5) + 105;
  }
  void setPriority(int bonus){                        //set new priority amounts, make sure it is within bounds
    if((initPriority + bonus) < 100)                //if not, force it within bounds
      priority = 100;
    else if((initPriority + bonus) > 150)
      priority = 150;
    else
      priority = initPriority + bonus;
  }
  void setProcessStart(int start){startTime = start;} //set process starting time
  void setTimeSlice(int t){timeSlice = t;}            //sets the time slice for a process
  void decrementTimeSlice(){timeSlice--;}             //decrement the timeslice by 1
  void setNumCpuBurst(int duration){numCpuBurst.push_back(duration);} //vector of cpu bursts with their duration
  void setNumIoBurst(int duration){numIoBurst.push_back(duration);}   //vector of io bursts with their duration
  void decrementNumCpuBurst(){numCpuBurst[0]--;}      //decrease the cpu burst by one
  void decrementNumIoBurst(){numIoBurst[0]--;}        //decrease the io burst by one
  void removeCpuBurst(){numCpuBurst.erase(numCpuBurst.begin());}  //cpu burst has reached 0, remove it from vector
  void removeIoBurst(){numIoBurst.erase(numIoBurst.begin());}     //io burst has reached 0, remove it from vector
private:
  int pid, startTime, endTime, initPriority, priority, timeSlice, totalCpuBurst, totalCpuTime, totalIoTime;
  vector<int> numCpuBurst;
  vector<int>numIoBurst;
};

/*calculates the time slice and priority for processes after their initial values*/
void calcPriorityTimeSlice(vector<Process> &process, int i){
  int bonus = 0;
  int x=0;
  if(process[i].getTotalCpuTime() < process[i].getTotalIoTime())
    bonus = (int)(((1 - (process[i].getTotalCpuTime() / (double)process[i].getTotalIoTime())) * (-10)) - 0.5);
  else
    bonus = (int)(((1 - (process[i].getTotalIoTime() / (double)process[i].getTotalCpuTime())) * 10) + 0.5);
        
  process[i].setPriority(bonus);
  x = (int)(((1 - (process[i].getPriority() / 150.0)) * 445) + 0.5 + 5);
  process[i].setTimeSlice(x);
    
}

/*calculates the first timeslice of a process*/
void calcFirstTimeSlice(Process process[], int pCount){
  int x;
  x = (int)(((1 - (process[pCount].getPriority() / 150.0)) * 445) + 0.5 + 5);
  process[pCount].setTimeSlice(x);
}

/*sort processes according to their starting time*/
void quickSort(Process arr[], int left, int right) {
  int i = left, j = right;
  Process tmp;
  int pivot = arr[(left + right) / 2].getStartTime();
 
  /* partition */
  while (i <= j) {
    while (arr[i].getStartTime() < pivot)
      i++;
    while (arr[j].getStartTime() > pivot)
      j--;
    if (i <= j) {
      tmp = arr[i];
      arr[i] = arr[j];
      arr[j] = tmp;
      i++;
      j--;
    }
  };
 
  /* recursion */
  if (left < j)
    quickSort(arr, left, j);
  if (i < right)
    quickSort(arr, i, right);
}

/*reads the input from the indirected file and stores the information to each
Process object. Info and calculations includes nice value, process ID, priority,
time slice, arrival time, number of cpu bursts, each cpu burst duration, each io 
burst duration. Finally sort info based on priority*/
void readInput(Process process[], int &processCount){           //read the input from the file until "***" is encountered
  vector<int> values;
  string inLine;
  int n;
  int j;
  getline(cin, inLine);
    
  while(true){
    stringstream stream(inLine);
    while(stream >> n){
      values.push_back(n);
    }
    for(int i = 0; i< values.size(); i++){
      if(i == 0){                                               //get nice value, set pid and priority
	process[processCount].setPid(processCount);
	process[processCount].setInitPriority(values[i]);
	process[processCount].setPriority(0);
	calcFirstTimeSlice(process, processCount);
      }
      else if(i == 1){                                           //arrival time
	//   startTime.push_back(values[i]);              
	process[processCount].setProcessStart(values[i]);
      }
      else if(i == 2){                                           //number of cpu bursts
	process[processCount].setTotalCpuBurst(values[i]);
      }
      else if(i >= 3){
	if(i % 2 == 1){                                         //CPU burst
	  process[processCount].setNumCpuBurst(values[i]);
	}
	else{                                                   //IO burst
	  process[processCount].setNumIoBurst(values[i]);
	}
      }
    }
    values.erase(values.begin(), values.end());
    processCount++;
    getline(cin, inLine);
    if(inLine == "***")
      break;
  }
  quickSort(process, 0, processCount-1);
  return;
}

/*find the lowest ready queue priority*/
int lowestActivePriority(vector<Process> &active){
  int lowest = 200, index = 0;
  for(int i=0; i<active.size(); i++){
    if(active[i].getPriority() < lowest){
      lowest = active[i].getPriority();
      index = i;
    }
  }
  return index;
}
/*print the finishing report for all processes which includes turn around time,
total cpu time, waiting time, and percentage of CPU utilization along with averages
of these for all processes*/
void finishReport(vector<Process> &finished){
  double cut = 0, avgTat = 0, avgWt = 0, avgCut = 0;
  int tat = 0, tct = 0, tit = 0, wt = 0, size = 0;
    
  cout << "\nENDING REPORT\n\n";
  for(int i=0; i<finished.size(); i++){
    tat = finished[i].getEndTime() - finished[i].getStartTime();
    tct = finished[i].getTotalCpuTime();
    tit = finished[i].getTotalIoTime();
    wt = tat - tct - tit;
    cut = (tct/(double)tat)*100.0;
        
    avgTat = avgTat + tat;
    avgWt = avgWt + wt;
    avgCut = avgCut + cut;
    cout << "Process <" << finished[i].getPid() << ">" << endl;
    cout << "Turn around time (TAT): " << tat << endl;
    cout << "Total CPU time (TCT): " << finished[i].getTotalCpuTime() << endl;
    cout << "Waiting time (WT): " << wt << endl;
    cout << setprecision(1) << fixed << "Percentage of CPU utilization time (CUT): " << cut << "%\n" << endl;
  }
    
  size = finished.size();
  avgTat = avgTat/size;
  avgWt = avgWt/size;
  avgCut = avgCut/size;
    
  cout << setprecision(3) << fixed << "Average Waiting Time: " << avgWt << endl;
  cout << setprecision(3) << fixed << "Average Turnaround Time: " << avgTat << endl;
  cout << setprecision(3) << fixed << "Average CPU Utilization: " << avgCut << "%" << endl;
  return;
}
int main(){
vector<Process> active, expired, io, finished, cpu;
Process startUp[MAX_PROCESS];
int processCount = 0, runningCount = 0, index = 0;
readInput(startUp, processCount);
runningCount = processCount;
int clock = 0;
while(true){
  index = 0;
        
  for(int i=0; i<processCount; i++){              //clock hit start time for a process, add the process to active from startup
    if(startUp[i].getStartTime() == clock){
      active.push_back(startUp[i]);
      cout << "[" << clock << "]" << " <" << active[active.size() -1].getPid() << "> " << "Enters ready queue (Priority: " <<
	active[active.size() -1].getPriority() << ", TimeSlice: " << active[active.size() -1].getTimeSlice() << ")" << endl;
      runningCount--;
    }
  }
        
  if(cpu.size() == 0 && active.size() > 0){       //CPU is empty and processes are in the ready queue, put highest priority into CPU
    index = lowestActivePriority(active);
    cpu.push_back(active[index]);
    active.erase(active.begin() + index);
    cout << "[" << clock << "]" << " <" << cpu[0].getPid() << "> Enters the CPU" << endl;
  }
        
  if(active.size() > 0){                          //If a process in the ready queue has lower priority than CPU process, pre-empt the cpu process
    index = lowestActivePriority(active);
    if(active[index].getPriority() < cpu[0].getPriority()){
      cout << "[" << clock << "] <" << active[index].getPid() << "> Preempts Process " << cpu[0].getPid() << endl;
      active.push_back(cpu[0]);
      cpu[0] = active[index];
      active.erase(active.begin() + index);
    }
  }
                
  if(cpu.size() > 0){                 //perform cpu 
    cpu[0].decrementTimeSlice();
    cpu[0].decrementNumCpuBurst();
    cpu[0].setTotalCpuTime();
  }
        
  for(int j=0; j<io.size(); j++){     //decrease ALL io time
    io[j].decrementNumIoBurst();
    io[j].setTotalIoTime();
  }
        
  if(cpu.size() > 0){                     //if the cpu is not empty
    if(cpu[0].getNumCpuBurst() == 0){   //if the cpu process exhausted its cpu burst
      cpu[0].removeCpuBurst();        
      if(cpu[0].getCpuBurstSize() == 0){  //if ALL cpu bursts are finished move to the Finished Queue
	cpu[0].setEndTime(clock+1);
	finished.push_back(cpu[0]);
	cout << "[" << clock << "] <" << cpu[0].getPid() << "> Finishes and moves to the Finished Queue" << endl;
      }
      else{
	cout << "[" << clock << "] <" << cpu[0].getPid() << "> Moves to the IO Queue" << endl;  //cpu burst done, process moves to io
	io.push_back(cpu[0]);
      }
      cpu.clear();
    }
    else{
      if(cpu[0].getTimeSlice() == 0){     //if current process timeslice is used, get new timeslice and priority and move to expired queue
	calcPriorityTimeSlice(cpu, 0);
	cout << "[" << clock << "] <" << cpu[0].getPid() << "> Finishes its time slice and moves to the " <<
	  "Expired Queue (Priority: " << cpu[0].getPriority() << ", TimeSlice: " << cpu[0].getTimeSlice() << ")" << endl;
	expired.push_back(cpu[0]);
	cpu.clear();
      }
    }
  }
       
  for(int i=0; i<io.size(); i++){         //if processes are in io
    if(io[i].getNumIoBurst() == 0){     //if current io burst is done
      io[i].removeIoBurst();
      if(io[i].getTimeSlice() == 0){  //if the io process' timeslice was done, get new timeslice and priority
	calcPriorityTimeSlice(io, i);
	expired.push_back(io[i]);
	cout << "[" << clock << "] <" << io[i].getPid() << "> Finishes IO and moves to the Expired Queue " <<
	  "(Priority: " << io[i].getPriority() << ", TimeSlice: " << io[i].getTimeSlice() << endl;
	io.erase(io.begin() + i);
      }
      /*process still had timeslice, move back to ready queue*/
      else{
	cout << "[" << clock << "] <" << io[i].getPid() << "> Finishes IO and moves to the Ready Queue" << endl;
	active.push_back(io[i]);        
	io.erase(io.begin() + i);           
      }
    }
  }
        
    /*ALL processes have finished everything */
    if(runningCount == 0 && active.size() == 0 && expired.size() == 0 && io.size() == 0 && cpu.size() == 0)
      break;
        
    /*The ready queue and cpu are empty, but expired has items, initiate queue swap*/    
    if(active.size() == 0 && cpu.size() == 0 && expired.size() > 0){
      active = expired;
      expired.clear();
      cout << "[" << clock << "] *** Queue Swap" << endl; 
    }
    clock++;
  }
  finishReport(finished); //print the finishing report
  return 0;
}

