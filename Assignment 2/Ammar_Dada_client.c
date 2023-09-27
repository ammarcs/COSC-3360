#include<stdio.h>
#include<netdb.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

//FUNCTION TO RESOLVE HOSTNAME TO IP ADDRESS
char* resolveIP(char* hostname){

	struct hostent *host;
	host = gethostbyname(hostname); 
	
	return inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));

}

//FUNCTION TO SET SERVER ADDRESS FOR UDP COMMUNICATION
void setServerAddress(struct sockaddr_in* server_addr, char* hostname, int port_number){

	(*server_addr).sin_family = AF_INET;
	(*server_addr).sin_addr.s_addr = inet_addr(resolveIP(hostname));
	(*server_addr).sin_port = htons(port_number);

}

//MAIN FUNCTION
int main(){

	//DECLARING REQUIRED VARIABLES
	struct sockaddr_in server_addr;
	char port_number[20], license_plate[20], input_buffer[256], hostname[256];
	int client_sock, msglength, server_length;
	
	//INPUTTING SERVER HOST NAME, PORT NUMBER AND LICENSE PLATE TO CHECK
	printf("Enter the server host name: ");
	scanf("%s",hostname);

	printf("Enter the server port number: ");
	scanf("%s",port_number);
		
	printf("Enter a license plate number: ");
	scanf("%s", license_plate);
	
	//CREATINNG UDP SOCKET FOR CLIENT
	client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	//CHECKING FOR ERRORS DURING SOCKET CREATION
	if( client_sock < 0){
	
		printf("Socket Creation Failed!!!\n");
		return 0;
	}

	//SETTING SERVER SIZE AND ADDRESS
	server_length = sizeof(server_addr);
	setServerAddress(&server_addr, hostname, atoi(port_number));

	//SENDING REQUEST TO SERVER TO CHECK LICENSE PLATE
	sendto(client_sock, license_plate, strlen(license_plate), MSG_CONFIRM, (const struct sockaddr*) &server_addr,server_length);
		
	//IF KILL MESSAGE SENT, DON'T EXPECT ANY RESPONSE
	if(strcmp(license_plate,"KILLSVC") == 0){
	
		close(client_sock);
	}

	//WAITING FOR SERVER RESPONSE
	else{	
		
 		msglength = recvfrom(client_sock, (char*)input_buffer, sizeof(input_buffer), MSG_WAITALL, ( struct sockaddr*) &server_addr, &server_length);
    		input_buffer[msglength] = '\0';
    		printf("%s\n",input_buffer);
    		close(client_sock);
    	}
		
	
	
}
