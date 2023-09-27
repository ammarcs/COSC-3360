#include<iostream>
#include<unistd.h>
#include<pthread.h>
using namespace std;

//Declaring Shared Global Variables
int time_to_raise, time_to_lower;
int drawbridgeStatus=1, number_of_cars=0, number_of_ships=0;
//Declaring Mutex
static pthread_mutex_t drawBridgeMutex;
//Declaring Conditional Variable
pthread_cond_t carCanGo_cond = PTHREAD_COND_INITIALIZER;

//For Passing Arguments to Threads
struct threadArgs{

	string vehicle_name;
	int crossing_time;

};

//Code for Car Thread
void* car(void* args){
	
	threadArgs* arguments = (threadArgs*)args;
	cout<<"Car "<<arguments->vehicle_name<<" arrives at the bridge.\n";
	//Waiting on Mutex
	pthread_mutex_lock(&drawBridgeMutex);
	number_of_cars+=1;
	//Waiting on Conditional Variable
	while(drawbridgeStatus==0)
		pthread_cond_wait(&carCanGo_cond, &drawBridgeMutex);
	cout<<"Car "<<arguments->vehicle_name<<" goes on the bridge.\n";
	//Sleep for Bridge Crossing Time
	sleep(arguments->crossing_time);
	cout<<"Car "<<arguments->vehicle_name<<" leaves the bridge.\n";
	//Release Mutex 
	pthread_mutex_unlock(&drawBridgeMutex);
	pthread_exit(NULL);
}

//Code for Ship Thread
void* ship(void* args){
	
	threadArgs* arguments = (threadArgs*)args;
	cout<<"Ship "<<arguments->vehicle_name<<" arrives at the bridge.\n";
	//Closing DrawBridge
	drawbridgeStatus = 0;
	cout<<"Bridge is closed to car traffic.\n";
	//Waiting on Mutex
	pthread_mutex_lock(&drawBridgeMutex);
	number_of_ships+=1;
	cout<<"Bridge can now be raised.\n";
	//Sleeping for Time to Raise Bridge
	sleep(time_to_raise);
	cout<<"Ship "<<arguments->vehicle_name<<" goes under the raised bridge.\n";
	//Sleep for Bridge Crossing Timeee
	sleep(arguments->crossing_time);
	cout<<"Ship "<<arguments->vehicle_name<<" is leaving.\n";
	//Sleeping for Time to Lower Bridge
	sleep(time_to_lower);
	cout<<"Bridge can now accommodate car traffic.\n";
	drawbridgeStatus = 1;
	//Signal Conditional Variable
	pthread_cond_signal(&carCanGo_cond);
	//Release Mutex
	pthread_mutex_unlock(&drawBridgeMutex);
	pthread_exit(NULL);
}

//Code for Main Function
int main(){

	//Declaring Necessary Variables
	int time,num_threads;
	threadArgs tArgs[500];
	pthread_t t_IDs[500];
	string vehicle_type, vehicle_name;
	
	//Initializing Mutex
	pthread_mutex_init(&drawBridgeMutex,NULL);
	
	//Reading Time Required to Raise and Lower Bridge
	cin>>vehicle_type;
	cin>>time_to_raise;
	cin>>time_to_lower;
	
	//Reading Data For Each Arriving
	for(num_threads=0; cin>>vehicle_type; num_threads++){
	
		cin>>vehicle_name>>time;
		sleep(time);
		cin>>time;
		tArgs[num_threads].vehicle_name = vehicle_name;
		tArgs[num_threads].crossing_time = time;
		//Creating Thread and Passing Arguments
		pthread_create(&t_IDs[num_threads], NULL, (vehicle_type=="Car")?car:ship, (void*)&tArgs[num_threads]);
	}
	
	//Joining all threads
	for(int i=0; i<num_threads; i++)
		pthread_join(t_IDs[i],NULL);
	
	//Printing Final Statistics
	cout<<"\n==>"<<number_of_cars<<" car(s) crossed the bridge.";
	cout<<"\n==>"<<number_of_ships<<" ship(s) under the raised bridge.\n";
}
