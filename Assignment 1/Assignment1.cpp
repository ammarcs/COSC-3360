#include<iostream>
#include<vector>
using namespace std;

//GLOBAL TIME
int global_clock=0;

//STRUCTURE FOR REGISTERING A COMPLETION EVENT
class EventRegister{
	
	public:
		
	int completion_time;
	string event_type;
	int process_id;
	
	EventRegister(){
		
		completion_time=0;
		event_type="";
		process_id=0;
		
	}
	
	EventRegister(int time, string t, int id){
		
		completion_time=time;
		event_type=t;
		process_id=id;
		
	}
	
};

//STRUCTURE OF INPUT TABLE LINE
class InputEntry{
	
	public:
		
		string input_operation;
		int input_parameter;
		
		InputEntry(){
			
			input_operation="";
			input_parameter=0;
			
		}
	
		InputEntry(string op, int pa){
			
			input_operation=op;
			input_parameter=pa;
			
		}	
};

//STRUCTURE FOR PROCESS TABLE LINE
class ProcessEntry{
	
	public:
		
	bool arrival;
	int id;
	string state;
	int start_line;
	int current_line;
	int end_line;

	ProcessEntry(int start, int end, int current, int i, bool arr){
		
		start_line = start;
		end_line = end;
		current_line = current;
		id = i;
		arrival = arr;
		
	}
		
};

//STRUCTURE OF QUEUE-NODE
class QueueNode{
	
	public:
		EventRegister queuedEvent;
		QueueNode* nextNode;
		
		QueueNode(){
			
			nextNode=NULL;
		}
		
		QueueNode(EventRegister d){
			
			queuedEvent = d;
			nextNode=NULL;
		}
		
};

//DEFINING A BI-FUNCTIONAL QUEUE (BOTH FCFS AND PRIORITY)

class Queue{
	
	QueueNode* frontNode;
	QueueNode* rearNode;
	string queueType;
	
	public:
		
		Queue(){
			
			frontNode = NULL;
			rearNode = NULL;
		}
		
		Queue(string t){
			
			frontNode = NULL;
			rearNode = NULL;
			queueType = t;
			
		}
		
		//FUNCTION -- PUSH ELEMENT AT APPROPRIATE POSITION IN QUEUE
		void Enqueue(EventRegister val){
			
			QueueNode* newNode = new QueueNode(val);
			
			if(notEmpty()){
				
				//FCFS ( SUCH AS READY QUEUE / SSD QUEUE / LOCK QUEUE)
				if(queueType=="FCFS"){
				
					rearNode->nextNode = newNode;
					rearNode = newNode;
				}
				
				//PRIORITY ( SUCH AS EVENT LIST -- BASED ON EVENT TIME)
				else if(queueType=="PRIORITY"){
					
					if(frontNode->queuedEvent.completion_time > val.completion_time){
						
						newNode->nextNode = frontNode;
						frontNode = newNode;
					}
						
					else{
						
					 	QueueNode* curr = frontNode;
		
						for(;curr->nextNode && curr->nextNode->queuedEvent.completion_time <= val.completion_time;)
							curr = curr->nextNode;
							
						newNode->nextNode = curr->nextNode;
						curr->nextNode = newNode;
					}	
				}
			}
			
			else	
				frontNode = rearNode = newNode;	
		}
		
		//FUNCTION -- POP ELEMENT FROM QUEUE'S FRONT
		EventRegister Dequeue(){
			
			if(!notEmpty()){
				
				EventRegister NullObject;
				return NullObject;
			}
			
			else{
				QueueNode* temp = frontNode;
				EventRegister val = frontNode->queuedEvent;
				frontNode = frontNode->nextNode;
				delete temp;
				rearNode = (!notEmpty()) ? NULL : rearNode;				
				return val;
				
			}

		}
		
		//FUNCTION -- CHECK IF QUEUE IS EMPTY OR NOT
		bool notEmpty(){
			
			if(frontNode==NULL || rearNode==NULL)
				return false;
				
			else
				return true;
				
		}
		
		//FUNCTION -- PRINT ALL ELEMENTS OF QUEUE (NEEDED SPECIFICALLY FOR READY QUEUE)
		void printQueue(){
			
			if(notEmpty()){
				
				cout<<"Ready Queue contains:"<<endl;
				
				QueueNode* curr = frontNode;
				
				for(;curr!=NULL;){
					
					cout<<"Process "<<curr->queuedEvent.process_id<<endl;
					curr = curr->nextNode;
					
				}
				cout<<endl;
			}
			
			else
				cout<<"Ready Queue is Empty."<<endl<<endl;	
		}
};

//STRUCTURE TO DEFINE PARAMETERS AND QUEUES REQUIRED FOR SCHEDULING
struct SchedulerParameters{
	
	Queue* ReadyQueue;
	Queue* SSDQueue;
	Queue* LockQueues;
	Queue* EventList;
	bool* Locks;
	bool SSD;
	vector<InputEntry> InputTable;
	vector<ProcessEntry> ProcessTable;
	int TotalCores;
	int AvailableCores;
	
};

//FUNCTION -- INITIALIZING ALL PARAMETERS FOR SCHEDULER
SchedulerParameters generateParameters(vector<InputEntry> input_table, vector<ProcessEntry> process_table, Queue* event_list, int cores){
	
	SchedulerParameters s_params = {new Queue("FCFS"), new Queue("FCFS"), new Queue[64], event_list, new bool[64], false, input_table, process_table, cores, cores};
	
	for(int i=0; i<64; i++){
		s_params.LockQueues[i] = Queue("FCFS");
		s_params.Locks[i] = false;
	}
	return s_params;
}

//FUNCTION -- PROCESS REQUEST CPU CORE
void core_req(SchedulerParameters* sp, int proc_id, int etime){
		
	EventRegister event(etime,"CPU",proc_id);
		
	//IF ALL CORES OCCUPIED
	if(sp->AvailableCores<=0){
		sp->ProcessTable[proc_id].state = "READY";
		sp->ReadyQueue->Enqueue(event);
	}
	
	//IF A CORE IS FREE
	else{				
		sp->ProcessTable[proc_id].state = "RUNNING";	
		event.completion_time += global_clock;
		sp->AvailableCores--;
		sp->EventList->Enqueue(event);
	}
		
}

//FUNCTION -- PROCESS REQUEST SSD
void ssd_req(SchedulerParameters* sp, int proc_id, int etime){
		
	EventRegister event(etime,"SSD",proc_id);
		
	//IF SSD BUSY
	if(sp->SSD){
		sp->ProcessTable[proc_id].state = "BLOCKED";
		sp->SSDQueue->Enqueue(event);
	}
	
	//IF SSD FREE
	else{
		event.completion_time += global_clock;
		sp->SSD=true;
		sp->ProcessTable[proc_id].state = "BLOCKED";
		sp->EventList->Enqueue(event);
	}
		
}

//FUNCTION -- PROCESS REQUEST SPECIFIC LOCK
void lock_req(SchedulerParameters* sp, int proc_id, int lock_num){
		
	EventRegister event(0,"LOCK",proc_id);
		
	//IF SPECIFIC LOCK IS ACQUIRED BY SOMEONE
	if(sp->Locks[lock_num]){
		sp->LockQueues[lock_num].Enqueue(event);
		sp->ProcessTable[proc_id].state = "BLOCKED";	
	}
		
	//IF LOCK IS AVAILABLE
	else{
		sp->Locks[lock_num]=true;
		event.completion_time = global_clock;
		sp->EventList->Enqueue(event);
	}
		
}
	
//FUNCTION -- PROCESS REQUESTS TO UNLOCK A SPECIFIC LOCK
void unlock_req(SchedulerParameters* sp, int lock_num){
		
	sp->Locks[lock_num] = false;

	//IF SOME PROCESS IS WAITING FOR THIS SPECIFIC LOCK
	if(sp->LockQueues[lock_num].notEmpty()){
		EventRegister event = sp->LockQueues[lock_num].Dequeue();
		lock_req(sp, event.process_id,lock_num);
	}
		
}

//FUNCTION -- PROCESS REQUESTS IO
void io_req(SchedulerParameters* sp, int proc_id, InputEntry command){
		
		sp->ProcessTable[proc_id].state = "BLOCKED";
		EventRegister event(global_clock+command.input_parameter, command.input_operation, proc_id);
		sp->EventList->Enqueue(event);
}
	
//FUNCTION -- DISPLAY THE PROCESSES AND STATUS IN PROCESS TABLE
void displayProcTable(SchedulerParameters* sp, int term=-1){
		
	cout<<"Process Table: \n";
	for(int i=0; i<sp->ProcessTable.size(); i++)			
		if(sp->ProcessTable[i].arrival == true && ( term==sp->ProcessTable[i].id || sp->ProcessTable[i].state!="TERMINATED"))
			cout<<"Process "<<sp->ProcessTable[i].id<<" is "<<sp->ProcessTable[i].state<<endl;	
		
}

//FUNCTION -- EXECUTES THE NEXT INSTRUCTION FOR REQUESTED PROCESS
void next_instruction(SchedulerParameters* sp, int proc_id){

	int cline = sp->ProcessTable[proc_id].current_line;
	int eline =  sp->ProcessTable[proc_id].end_line;
	
	if(cline<eline){
				
		sp->ProcessTable[proc_id].current_line++;
		InputEntry instruct = sp->InputTable[cline+1];
		string instruct_type = instruct.input_operation;
		
		//IF PROCESS NEXT INSTRUCT IS ACQUIRING A LOCK
		if(instruct_type == "LOCK")	
			lock_req(sp, proc_id, instruct.input_parameter);
			
		//IF PROCESS NEXT INSTRUCT IS CPU CORE REQUEST
		else if(instruct_type == "CPU")
			core_req(sp, proc_id,instruct.input_parameter);	
			
		//IF PROCESS NEXT INSTRUCT IS RELEASING A LOCK
		else if(instruct_type == "UNLOCK"){
			unlock_req(sp, instruct.input_parameter);
			next_instruction(sp, proc_id);
		}
		
		//IF PROCESS NEXT INSTRUCT IS SSD
		else if(instruct_type == "SSD")	
			ssd_req(sp, proc_id, instruct.input_parameter);
		
		//IF PROCESS NEXT INSTRUCT IS INPUT IO
		else if(instruct_type == "INPUT")
			io_req(sp, proc_id, instruct);
		
		//IF PROCESS NEXT INSTRUCT IS OUTPUT IO
		else if(instruct_type == "OUTPUT")	
			io_req(sp, proc_id, instruct);
		
		//IF PROCESS TERMINATES
		else if(instruct_type == "END"){
			
			sp->ProcessTable[proc_id].state = "TERMINATED";
			cout<<"Process "<<proc_id<<" terminates at time "<<global_clock<<" ms"<<endl;
			cout<<"Current number of busy cores "<<sp->TotalCores-sp->AvailableCores<<endl;
			displayProcTable(sp, proc_id);
			sp->ReadyQueue->printQueue();		
		}
	}
}

//FUNCTION -- START THE SIMULATION FOR SCHEDULER
void startScheduler(SchedulerParameters sp){
	
	//MAIN LOOP UNTIL ALL EVENTS COMPLETED
	while(sp.EventList->notEmpty()){
		
		//POP TOPMOST EVENT BASED ON TIME
		EventRegister nextEvent = sp.EventList->Dequeue();
		global_clock = nextEvent.completion_time;
		int proc_id = nextEvent.process_id;
		string nextEvent_type = nextEvent.event_type;
		
		//IF NEXT EVENT IS PROCESS START
		if(nextEvent_type=="START"){
			
			sp.ProcessTable[proc_id].state = "READY";	
			sp.ProcessTable[proc_id].arrival = true;
			cout<<"Process "<<proc_id<<" starts at time "<<global_clock<<" ms"<<endl;
			cout<<"Current number of busy cores "<<sp.TotalCores-sp.AvailableCores<<endl;
			displayProcTable(&sp);
			sp.ReadyQueue->printQueue();
		}
			
		//IF NEXT EVENT IS SSD COMPLETION
		else if(nextEvent_type=="SSD"){
				
			if(!sp.SSDQueue->notEmpty())
				sp.SSD=false;
				
			else{
				EventRegister event = sp.SSDQueue->Dequeue();
				event.completion_time = event.completion_time+global_clock;
				sp.ProcessTable[event.process_id].state = "BLOCKED";
				sp.EventList->Enqueue(event);
			}
				
		}
	
		//IF NEXT EVENT IS CPU COMPLETION
		else if(nextEvent_type=="CPU"){
			
			if(!sp.ReadyQueue->notEmpty())
				sp.AvailableCores++;
				
			else{
				EventRegister event = sp.ReadyQueue->Dequeue();
				sp.ProcessTable[event.process_id].state = "RUNNING";
				event.completion_time = event.completion_time+global_clock;
				sp.EventList->Enqueue(event);
			}
		}
		
		//FOR ALL EVENTS
		next_instruction(&sp,proc_id);	
	}
}

//MAIN FUNCTION -- IO REDIRECTION AND POPULATING INPUT/PROCESS TABLES
int main(){
	
	Queue* event_list = new Queue("PRIORITY");
	vector<InputEntry> InputTable;
	vector<ProcessEntry> ProcessTable;	
	string val1;
	int val2;

	//READING INPUT INTO INPUT TABLE (IO REDIRECTION)
	while(cin>>val1){
		
		if(val1=="END")
			val2=0;
		else
			cin>>val2;
		
		InputEntry i(val1,val2);
		InputTable.push_back(i);	
						
	}
		
	//FILLING PROCESS TABLE
	int pro_id=0,last_s,last_e;
	for(int i=0; i<InputTable.size(); i++){
		
		if(InputTable[i].input_operation=="END"){
			last_e=i;
			ProcessEntry p(last_s, last_e, last_s, pro_id++, false);
			ProcessTable.push_back(p);
		}
			
		else if(InputTable[i].input_operation=="START"){
			last_s = i;
			EventRegister en(InputTable[i].input_parameter,"START",pro_id);
			event_list->Enqueue(en);
		}
	}
	
	//INITIALIZING SCHEDULER
	SchedulerParameters sp = generateParameters(InputTable, ProcessTable, event_list, InputTable[0].input_parameter);
	
	//STARTING SCHEDULER
	startScheduler(sp);	
}
