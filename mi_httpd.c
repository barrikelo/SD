#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

int s; /* socket */
char respuesta[1024];
int maxClientes=10;
char DirectoryIndex[1024]="index.html";

int abrir_socket();

void enlazar_a_puerto(int , char*);

int escuchando(int, int);

void aceptando_conexion(int);

void cabeceras(char*, char*);

void hacerGET(char*,char*);

void hacerHEAD(char* ,char* );

void hacerDELETE(char* ,char* );

void hacerPUT(char* ,char* );

void leerConfig(char* );

int enviar(int ,char* );



int main (int argc, char *argv[])
{  
	char *servidor_puerto="8880";
	char mensaje[1024];
	struct sockaddr_in dir_servidor, dir_cliente;
	unsigned int long_dir_cliente;
	int s2;
	int n, enviados, recibidos;
	int proceso,i;
															    char directorio[100]="/home/barri/Escritorio";
 	strcpy(respuesta,"\n");
	/* Comprobar los argumentos */
	if (argc < 1)
	{
		fprintf(stderr, "Error. Debe indicar el puerto del servidor\r\n");
		fprintf(stderr, "Sintaxis: %s <puerto>\n\r", argv[0]);
		fprintf(stderr, "Ejemplo : %s 8574\"\n\r", argv[0]);
		return 1;
	}

	/* Tomar los argumentos */	
	/** Compruebo los argumentos el puerto si nos pasa uno y el archivo de configuracion */
	if(argv[1]!=NULL)
	  servidor_puerto = argv[1];
	
	for(i=0;i<argc;i++){
	  if(strcmp(argv[i],"-")){
	    i+=2;
	    if(i<argc){
	      if(strcmp(argv[i],"c")){
		i++;
		leerConfig(argv[i]);
		i++;
	      }
	    }
	  }
	}
	/**** Paso 1: Abrir el socket ****/

	s = abrir_socket();

	/**** Paso 2: Establecer la dirección (puerto) de escucha ****/

	enlazar_a_puerto(s,servidor_puerto); 

	/**** Paso 3: Preparar el servidor para escuchar ****/

	escuchando(s,maxClientes);

	/**** Paso 4: Esperar conexiones ****/


	while (1)
	{
		fprintf(stderr, "Esperando conexión en el puerto %s...\n\r", servidor_puerto);
		long_dir_cliente = sizeof (dir_cliente);
		s2 = accept (s, (struct sockaddr *)&dir_cliente, &long_dir_cliente);
		/* s2 es el socket para comunicarse con el cliente */
		/* s puede seguir siendo usado para comunicarse con otros clientes */
		if (s2 == -1)
		{
			break; /* salir del bucle */
		}
		/* crear un nuevo proceso para que se pueda atender varias peticiones en paralelo */
		proceso = fork();
		if (proceso == -1) exit(1);
		if (proceso == 0) /* soy el hijo */
		{
			close(s); /* el hijo no necesita el socket general */

			/**** Paso 5: Leer el mensaje ****/

			n = sizeof(mensaje);
			recibidos = read(s2, mensaje, n);
			if (recibidos == -1)
			{
				fprintf(stderr, "Error leyendo el mensaje\n\r");
				exit(1);
			}
			mensaje[recibidos] = '\0'; /* pongo el final de cadena */
			/**Anyadido por nosotros*/
			printf("Mensaje [%d]: %s\n\r", recibidos, mensaje);
			  char *argu0, *argu1, *argu2;
			  //printf("Split \"%s\"\n", mensaje);
			  
			  argu0 = strtok(mensaje, " ");	//metodo
			  //printf("%s\n", argu0);
			  
			  if(argu0!=NULL)
			    argu1 = strtok(NULL, " ");	//recurso
			  //printf("%s\n", argu1);
			  
			  if(argu1!=NULL)
			    argu2 = strtok(NULL, " ");	//version

	printf("%s\n", argu2);


			if(strcmp(argu0,"GET")==0){
			  char *dir=directorio;
			  strcat(dir,"/");
			  strcat(dir,argu1);
			  hacerGET(dir,argu2);
			}else if(strcmp(argu0,"HEAD")==0){
			  char *dir=directorio;
			  strcat(dir,"/");
			  strcat(dir,argu1);
			  hacerHEAD(dir,argu2);
			}else if(strcmp(argu0,"DELETE")==0){
			  char *dir=directorio;
			  strcat(dir,"/");
			  strcat(dir,argu1);
			  hacerDELETE(dir,argu2);
			}else if(strcmp(argu0,"PUT")==0){
			  char *dir=directorio;
			  strcat(dir,"/");
			  strcat(dir,argu1);
			  hacerPUT(dir,argu2);
			}else{
			  strcpy(respuesta,"\n405 method not allowed\n");
			}
			
			/**** Paso 6: Enviar respuesta ****/
			enviar(s2,respuesta);
						

			close(s2);
			exit(0); /* el hijo ya no tiene que hacer nada */
		}
		else /* soy el padre */
		{
			close(s2); /* el padre no usa esta conexión */
		}

	}

	/**** Paso 7: Cerrar el socket ****/
	close(s);
	printf("Socket cerrado\n\r");
	return 0;
}

void cabeceras(char* argu1, char* argu2){
  strcat(respuesta,"Date: ");
  time_t fecha;
  time(&fecha);
  strcat(respuesta,ctime(&fecha));
  strcat(respuesta,"Server: 127.0.0.1");
  strcat(respuesta,"\nLast-modified: ");
  strcat(respuesta,"\nContent-length: ");
  strcat(respuesta,"\nContent-type: html/txt");
  strcat(respuesta,"\nLocation: ");
  strcat(respuesta,argu1);
  strcat(respuesta,"\n\n");
}

void hacerGET(char* argu1,char* argu2){
  FILE * fichero,*fi;
  char car; int cont=0;
  char c[2];
  c[1]='\0';
  fichero=fopen(argu1,"r");
  strcat(respuesta,argu2);
  strcat(respuesta," ");
  if(strcmp(argu2,"HTTP/1.1")>=0){
      if(fichero!=NULL){
	strcat(respuesta,"200 OK\n");
	cabeceras(argu1,argu2);
	do{
	  car = fgetc(fichero);
	  c[0]=car;
	  strcat(respuesta,c);
	  cont++;
	}while (car != EOF);
	respuesta[strlen(respuesta)-1]='\0';
      }else{
	strcat(respuesta,"404 NOT FOUND\n");
	cabeceras(argu1,argu2);
      }
  }else {
    strcat(respuesta,"505 HTTP version not Supported\n");
    cabeceras(argu1,argu2);
  }
      if(fichero!=NULL)
	fclose(fichero);
  //strcpy(respuesta,"<html>Hola</html>\0");
}

void hacerHEAD(char* argu1,char* argu2){
  FILE * fichero,*fi;
  char car; int cont=0;
  char c[2];
  c[1]='\0';
  fichero=fopen(argu1,"r");
  strcat(respuesta,argu2);
  strcat(respuesta," ");
  if(strcmp(argu2,"HTTP/1.1")>=0){
    
      if(fichero!=NULL){
	strcat(respuesta,"200 OK\n");
	cabeceras(argu1,argu2);
	respuesta[strlen(respuesta)-1]='\0';
      }else{
	strcat(respuesta,"404 NOT FOUND\n");
	cabeceras(argu1,argu2);
      }
  }else {
    strcat(respuesta,"505 HTTP version not Supported\n");
    cabeceras(argu1,argu2);
  }
      if(fichero!=NULL)
	fclose(fichero);
  //strcpy(respuesta,"<html>Hola</html>\0");
}

void hacerDELETE(char* argu1,char* argu2) {
	int devuelve = -1;
	
	char filename[strlen(argu1)];
	strcpy(filename, argu1);
	strcat(respuesta,argu2);
	strcat(respuesta," ");
	if(strcmp(argu2,"HTTP/1.1")>=0){
	    
	    devuelve = remove(filename);
	    
	    if(devuelve == 0) {
		    	strcat(respuesta,"200 OK\n");
	    }
	    else {
		    strcat(respuesta,"404 NOT FOUND\n");
	    }
	}else{
	  strcat(respuesta,"505 HTTP version not Supported\n");
	  cabeceras(argu1,argu2);
	}
}

void hacerPUT(char* argu1,char* argu2){
	FILE *fichero;
	 
	  strcat(respuesta,argu2);
	  strcat(respuesta," ");
	
	  if(strcmp(argu2,"HTTP/1.1")>=0){
		fichero = fopen(argu1, "w");
		if(fichero!=NULL){
		  strcat(respuesta,"201 Created\n");
		  cabeceras(argu1,argu2);
		}else{
		  strcat(respuesta,"403 forbidden\n");
		  cabeceras(argu1,argu2);
		}
	  }else{
	  strcat(respuesta,"505 HTTP version not Supported\n");
	  cabeceras(argu1,argu2);
	  }
  
    	if(fichero != NULL) {
		fclose(fichero);
	}
}


/** metodo para leer el fichero de configuracion */

void leerConfig(char* filename){		
  FILE * fichero;
  char cadena[100];
  fichero = fopen(filename,"r");
  
  if(fichero==NULL){
    printf("ERROR al abrir fichero\nAnyadir envio de ERROR\n");
  }else{
    if( fgets(cadena, 100 , fichero) != NULL ){
      maxClientes=atoi(cadena);
      if( fgets(cadena, 100 , fichero) != NULL )
	strcpy(DirectoryIndex,cadena);
    }
  }
  if(fichero!=NULL)
    fclose(fichero);
}

int enviar(int s2,char* respuesta){
  int n,enviados;
 	n = strlen(respuesta);
	printf("Enviar respuesta [%d bytes]: %s\n\r", n, respuesta);
	enviados = write(s2, respuesta, n);//le envia al cliente lo que escribe en el socket
	printf("Numero de bytes enviados: %d\n", enviados);
	if (enviados == -1 || enviados < n){
		fprintf(stderr, "Error enviando la respuesta (%d)\n\r",enviados);
		close(s);
		return 1;
	}

	printf("Respuesta enviada\n\r");
 
}


int abrir_socket(){

	int sock=socket(PF_INET,SOCK_STREAM,0); //creo el socket 
      
	if(sock==-1){
		fprintf(stderr,"Error de apertura de socket\n\r");
	 }              
    	return sock;
  }


void enlazar_a_puerto(int listener, char* puertodefinido){

	  
	  struct sockaddr_in server_addr;
	  server_addr.sin_family=AF_INET;
	  server_addr.sin_port=htons(atoi(puertodefinido));
	  server_addr.sin_addr.s_addr=htonl(INADDR_ANY); /*significa cualquier ip del servidor*/

  

	  int c= bind(listener, (struct sockaddr *) &server_addr, sizeof(server_addr)); //Le pasamos el socket, direccion de memoria de la 	estructura, tamanyo
  
	    if(c==-1){
	      fprintf(stderr,"No se puede enlazar al puerto: dirección ya está en uso\n\r");
	      close(listener);
	    }

	  printf("Establecido el puerto de escucha\n\r");
  }


//FUNCIÓN LISTEN

int escuchando( int listener, int numero){
  
    int c= listen(listener,numero);
    
      if(c==-1){
	fprintf(stderr,"No es posible escuchar en ese puerto\n\r"); //cliente 11
	close(listener);
	return 1;
    }
	    
	  printf("Enlazado a puerto\n");
  }

