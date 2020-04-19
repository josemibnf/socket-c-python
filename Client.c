#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int main(){
//Datos de configuracion

	FILE* file;
	file = fopen("client.cfg", "r");
	
	char nom[10];
	fgets(nom,5,file);      //Almacena los 4 primeros caracteres, para poder almacenar solo apartir del 5. 
    fgets(nom,10,file);		//Sobreescribe los siguientes caracteres (hasta \n) encima de los cuatro primeros.
    printf("%s",nom);

    char mac[20];
    fgets(mac,5,file);
    fgets(mac,20,file);
    printf("%s",mac); 

    char server[20];
    fgets(server,8,file);
    fgets(server,20,file);
    printf("%s",server);

    char server_port[20];
    fgets(server_port,13,file);
    fgets(server_port,5,file);
    printf("%s\n",server_port);


//Registro en el servidor
	int n=3, t=2, m=4, p=8, s=5, q=3;
    
	//Se crea el socket UDP
    int Descriptor;
	Descriptor = socket(AF_INET, SOCK_DGRAM, 0);  //SOCK_DGRAM porque es UDP, TCP seria SOCK_STREAM.
	if(Descriptor==-1){
		printf("Error socket\n");
		die("socket");
	}
	//Asociamos el socket a un puerto
	struct sockaddr_in Direccion; //direccion cliente
	struct hostent *host;

	struct servent *Puerto=NULL;
	Puerto=getservbyname(server,"udp");

	Direccion.sin_family=AF_INET;
	Direccion.sin_port=Puerto->s_port;
	Direccion.sin_addr.s_addr=INADDR_ANY; 		
	
	if (bind (Descriptor, (struct sockaddr *)&Direccion, sizeof (Direccion)) == -1) { 
    	printf ("Error bind\n");
		die("bind"); 
	} 

	//Enviar registro al servidor
	struct sockaddr from;	//direccion servidor
	char* datos;  	//datos del servidor
	char estat[20] = "DISCONNECTED";  //estado
	char* registre_req = strcat("0x00"," "); 
		strcat(registre_req, nom); strcat(registre_req," ");
		strcat(registre_req, mac); strcat(registre_req, " ");
		strcat(registre_req, 0);
	bool fin = false;

	int N=0, M=2, Q=0, T=t;
	while(fin == false){
		if (sendto(Descriptor, (char *)&registre_req, sizeof(registre_req), 0, (struct sockaddr *)&Direccion, sizeof(Direccion))==-1){
			printf("Error sendTo\n");
			exit(2);
		}
		estat[20]="WAIT_REQ";

		if(recvfrom(Descriptor, (char *)&datos, sizeof(datos), 0, (struct sockaddr *)&from, sizeof(from)) == -1){
			printf("Error recvfrom\n");
			exit(2);
		}
		datos= strtok(datos, " ");
		if(datos[0]=="0x01"){
			//Registro aceptado ACK
			estat[20]="REGISTERED";
			fin=true;
		}
		else if(datos[0]=="0x02"){
			//Denegacion de registro NACK
			sleep(T);
			N=0; M=2; T=t; Q++;
			if (Q == q){
				print("No se ha podido conectar con el servidor\n");
				exit(2);
			}
		}
		else if (datos[0] == "0x03"){
			//Rechazo de registro REJ
			printg("Peticion de registro rechazada\n");
			estat[20]="DISCONNECTED";
			fin=true;
		}
		else if (datos[0] == "0x09"){
			//ERROR
			printf("Error recivido del servidor\n");
			exit(2);
		}
		else{
			sleep(T);
			N++;
			if(N>=n){
				T=M*T; 
				if(M<m){M++;}
				if(N=p){sleep(s); N=0; M=2; T=t; Q++;};
				if(Q==q){
					print("No se ha podido conectar con el servidor\n");
					exit(2);
				}
			}
		}
	}

	//Mantener comunicacion periodica
	int r=3; int u=3; int U1=0; int U2=0;
	if(estat[20]=="REGISTERED"){
		char *paquetTo = strcat("0x10", " ");
			strcat(paquetTo, nom);
			strcat(paquetTo, " ");
			strcat(paquetTo, mac);
			strcat(paquetTo, " ");
			strcat(paquetTo, 0);
		SetTimeout(mantenerComunicacion(Descriptor, Direccion, paquetTo, datos, from, estat, U1, U2, u), r);

	}


}



void mantenerComunicacion(int Descriptor, struct sockaddr_in Direccion, char *paquetTo, char *datos, struct sockaddr from, char estat[20], int U1, int U2, int u)
{
	char *paqueteFrom;
	if (sendto(Descriptor, (char *)&paquetTo, sizeof(paquetTo), 0, (struct sockaddr *)&Direccion, sizeof(Direccion)) == -1){
		printf("Error sendTo\n");
		exit(2);
	}
	if (recvfrom(Descriptor, (char *)&paqueteFrom, sizeof(paqueteFrom), 0, (struct sockaddr *)&from, sizeof(from)) == -1){
		printf("Error recvfrom\n");
		exit(2);
	}
	//COGER SOLO PAQUETES ACK, NO NACK
	//comprobacion de datos
	paqueteFrom = strtok(paqueteFrom, " ");
	U2++;
	datos[0]=0x11;
	if(estat[20=="ALIVE"] && paqueteFrom[0]==0x13){
		estat[20]=="DISCONNECTED";
		U1=0; U2=0;
	}
	if(paquetesIguales(datos, paqueteFrom)){
		estat[20]="ALIVE";
		U1++; U2++;
	}
	if(U2==u && U1==0){ 
		estat[20]="DISCONNECTED";
		U1=0; U2=0;
	}
	return;
}

bool paquetesIguales(char *paquete1, char *paquete2){
	for (int i=0; i<4; i++){
		if(paquete1[i] != paquete2[i]){
			return false;
		}
	}
	return true;
}