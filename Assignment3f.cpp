#include<iostream>
#include<unistd.h>
#include<pthread.h>
using namespace std;


int time_to_raise, time_to_lower;
int drawbridgeStatus=1, number_of_cars=0, number_of_ships=0;
static pthread_mutex_t drawBridgeMutex;
pthread_cond_t carCanGo_cond = PTHREAD_COND_INITIALIZER;

struct threadArgs{

	string vehicle_name;
	int crossing_time;

};

void* car(void* args){
	
	threadArgs* arguments = (threadArgs*)args;
	cout<<"Car "<<arguments->vehicle_name<<" arrives at the bridge.\n";
	pthread_mutex_lock(&drawBridgeMutex);
	number_of_cars+=1;
	while(drawbridgeStatus==0)
		pthread_cond_wait(&carCanGo_cond, &drawBridgeMutex);
	cout<<"Car "<<arguments->vehicle_name<<" goes on the bridge.\n";
	sleep(arguments->crossing_time);
	cout<<"Car "<<arguments->vehicle_name<<" leaves the bridge.\n";
	pthread_mutex_unlock(&drawBridgeMutex);
	pthread_exit(NULL);
}

void* ship(void* args){
	
	threadArgs* arguments = (threadArgs*)args;
	cout<<"Ship "<<arguments->vehicle_name<<" arrives at the bridge.\n";
	drawbridgeStatus = 0;
	cout<<"Bridge is closed to car traffic.\n";
	pthread_mutex_lock(&drawBridgeMutex);
	number_of_ships+=1;
	cout<<"Bridge can now be raised.\n";
	sleep(time_to_raise);
	cout<<"Ship "<<arguments->vehicle_name<<" goes under the raised bridge.\n";
	sleep(arguments->crossing_time);
	cout<<"Ship "<<arguments->vehicle_name<<" is leaving.\n";
	sleep(time_to_lower);
	cout<<"Bridge can now accommodate car traffic.\n";
	drawbridgeStatus = 1;
	pthread_cond_signal(&carCanGo_cond);
	pthread_mutex_unlock(&drawBridgeMutex);
	pthread_exit(NULL);
}

int main(){

	int time,num_threads;
	threadArgs tArgs[500];
	pthread_t t_IDs[500];
	string vehicle_type, vehicle_name;
	
	pthread_mutex_init(&drawBridgeMutex,NULL);
	
	cin>>vehicle_type;
	cin>>time_to_raise;
	cin>>time_to_lower;
	
	for(num_threads=0; cin>>vehicle_type; num_threads++){
	
		cin>>vehicle_name>>time;
		sleep(time);
		cin>>time;
		tArgs[num_threads].vehicle_name = vehicle_name;
		tArgs[num_threads].crossing_time = time;
		pthread_create(&t_IDs[num_threads], NULL, (vehicle_type=="Car")?car:ship, (void*)&tArgs[num_threads]);
	}
	
	for(int i=0; i<num_threads; i++)
		pthread_join(t_IDs[i],NULL);
	
	cout<<"\n==>"<<number_of_cars<<" car(s) crossed the bridge.";
	cout<<"\n==>"<<number_of_ships<<" ship(s) under the raised bridge.\n";
}
