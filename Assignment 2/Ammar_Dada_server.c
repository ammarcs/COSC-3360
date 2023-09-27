#include<stdio.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdbool.h>
#include<string.h>

//FUNCTION TO SEARCH LICENSE PLATE IN DATABASE/FILE
int searchInDatabase(char* license_plate, char* ficlient_lengthame){

	char readString[20];
	FILE* file_ptr = fopen(ficlient_lengthame,"r");
	
	//IF FILE NOT FOUND
	if(file_ptr==NULL)
		return -1;
	
	
	for(;fscanf(file_ptr,"%s\n",readString)!=EOF;){
	
		//IF LICENSE PLATE FOUND
		if(!strcmp(license_plate,readString))
			return 1;
	}
	
	//IF LICENSE PLATE NOT FOUND
	return 0;

}

//FUNCTION TO RESOLVE HOSTNAME TO IP ADDRESS
char* resolveIP(char* hostname){

	struct hostent *host;
	host = gethostbyname(hostname); 
	
	return inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));

}

//FUNCTION TO SET SERVER ADDRESS FOR UDP COMMUNICATION
void setServerAddress(struct sockaddr_in* server_addr, int port_number){

	(*server_addr).sin_family = AF_INET;
	(*server_addr).sin_addr.s_addr = inet_addr(resolveIP("localhost"));
	(*server_addr).sin_port = htons(port_number);

}

//MAIN FUNCTION
int main(){
	
	//DECLARING REQUIRED VARIABLES
	struct sockaddr_in server_addr, client_addr;
	char database[100], port_number[20], input_buffer[256], output_buffer[256];
	int server_sock, msglength, client_length, find;
	
	//INPUTTING STOLEN CAR FILENAME CONTAINING RECORDS AND PORT NUMBER 
	printf("Enter today’s stolen car DB name: ");
	scanf("%s",database);

	printf("Enter the server port number: ");
	scanf("%s",port_number);
	
	//CREATINNG UDP SOCKET FOR SERVER
	server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//CHECKING FOR ERRORS DURING SOCKET CREATION
	if(server_sock < 0){
	
		printf("Socket Creation Failed!!!\n");
		return 0;
	}

	//SETTING SERVER ADDRESS
	setServerAddress(&server_addr, atoi(port_number));

	//BIND ADDRESS TO SOCKET
	bind(server_sock, (const struct sockaddr *)&server_addr, sizeof(server_addr));

	client_length = sizeof(client_addr);

	//LOOP TO RECIEVE REQUESTS FROM CLIENTS REPETETIVELY
	while(true){
		
		//RECIEVE REQUEST FROM CLIENT TO CHECK LICENSE PLATE
 		msglength = recvfrom(server_sock, (char *)input_buffer, sizeof(input_buffer), MSG_WAITALL, ( struct sockaddr *) &client_addr, &client_length);		
		input_buffer[msglength] = '\0';
		
		//IF KILL REQUEST, TERMINATE
		if(strcmp(input_buffer,"KILLSVC")==0){
			close(server_sock);
			return 1;
		
		}
		
		//ELSE SEARCH FOR PLATE IN DATABASE/FILE
		find = searchInDatabase(input_buffer, database);
		
		//IF LICENSE PLATE FOUND
		if(find==1){
		
			strcpy(output_buffer,input_buffer);
			strcat(output_buffer,": Reported as stolen");
			printf("%s\n",output_buffer);
			sendto(server_sock, output_buffer, strlen(output_buffer), MSG_CONFIRM, (const struct sockaddr*) &client_addr,client_length);
		}
		
		//IF LICENSE PLATE NOT FOUND
		else if(find==0){
		
			strcpy(output_buffer,input_buffer);
			strcat(output_buffer,": Not in the database");
			printf("%s\n",output_buffer);
			sendto(server_sock, output_buffer, strlen(output_buffer), MSG_CONFIRM, (const struct sockaddr*) &client_addr,client_length);
		}
		
		//IF FILE NOT FOUND
		else if(find==-1){
		
			printf("Database Not Found!!!\n");
		}

	}
	
}
