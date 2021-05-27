#include <stdio.h>
#include <stdlib.h>

void huffman_decoder(FILE * ent, FILE * sal);

struct node{
	unsigned int w; // Peso
	unsigned short p; // Padre
	unsigned short c0, c1; // 0 / 1
	unsigned char cl; // Tamaño del código
};

int main(int argv, char** argc){
	if(argv != 4){
		printf("Uso: %s d /ruta/entrada/archivo /ruta/salida/archivo\n", argc[0]);
		return 0;
	}

	FILE * ent, * sal;//Declaración de apuntadores de archivo.

	if(!(ent = fopen(argc[2], "rb"))) {//Argumento ruta de archivo de entrada
		perror("Error al abrir archivo de entrada");
		return -1;

	}

    if(!(sal = fopen(argc[3], "wb"))){//Argumento ruta de archivo de salida
		perror("Error al abrir archivo de salida");
		return -1;
	}
	if(*argc[1] == 'd')//Argumento d de codificación
		huffman_decoder(ent, sal);
	else
		printf("Uso: %s d /ruta/entrada/archivo /ruta/salida/archivo\n", argc[0]);

	fclose(ent);
	fclose(sal);
	return 0;
}

void huffman_decoder(FILE * ent, FILE * sal){
	int i, j;       //Variables auxiliares
	size_t length;  //Tamaño del largo
	size_t rcount;  //Tamaño del conteo

	//Construcción de la tabla de frecuencias
	unsigned int freqTable[256] = {0}; // Contador para frecuencias por palabra.
	unsigned char redirTab[256]; // Tabla inversa de redirección.
	double utime0, stime0, wtime0, utime1, stime1, wtime1; //Variables para medicion de tiempos
	//******************************************************************
	//Iniciar el conteo del tiempo para las evaluaciones de rendimiento
	//******************************************************************
    uswtime(&utime0, &stime0, &wtime0);
	if(fread(&length, sizeof(size_t), 1, ent) < 1){ //Lectura del tamaño del archivo de entrada
		printf("No se puede decodificar el archivo.\n");
		exit(-1);
	}

	if(fread(freqTable, sizeof(unsigned int), 256, ent) < 256){//Lectura del tamaño de la tabla de frecuencias 
		printf("No se puede decodificar el archivo.\n");
		exit(-1);
	}

	/// Construcción del árbol
	struct node arbol[512000] = {0}; // Memoria para alojar el árbol
	unsigned short nodoLibre[256];  // Lista de sub arboles libres
	unsigned short a, b; // a = nodo más pequeño; b = segundo nodo más pequeño

	j = 0;
	for(i = 0; i < 256; i++){// Inicializa hojas y libera nodos
		if(freqTable[i]){
			redirTab[j] = i;//Aqui es donde se meten los caracteres
			nodoLibre[j] = j;//En un nodo individual 
			arbol[j].w = freqTable[i];
			j++;
		}
	}

	unsigned short inode = j; // Siguiente nodo libre
	unsigned short lfnode = j - 1; // ultima posición ocupada en nodoLibre

	while(lfnode > 0){// Iterar mientras que el nodo izquierdo libre sea mayor a 0 hasta que el árbol este completo
		if(arbol[nodoLibre[0]].w < arbol[nodoLibre[1]].w){// Inicializar a y b, si 
			a = 0;//nodo más pequeño
			b = 1;//segundo nodo más pequeño
		}else{
			b = 0;
			a = 1;
		}
		for(i = 2; i <= lfnode; i++){// Encontrar los 2 nodos más pequeños
			unsigned int w = arbol[nodoLibre[i]].w;
			if(w < arbol[nodoLibre[a]].w){//Buscamos cual nodo es más pequeño
				b = a;                      // y realizamos asignaciones
				a = i;                      //para llevar un control de los nodos
			}else{
				if(w < arbol[nodoLibre[b]].w) b = i;
			}
		}

		// Llenar datos

		unsigned short _a = nodoLibre[a];
		unsigned short _b = nodoLibre[b];
		arbol[_a].p = inode;//nodo padre a
		arbol[_b].p = inode;//nodo padre b
		arbol[inode].c0 = arbol[_a].cl <= arbol[_b].cl ? _a : _b;//nodo interno 0 (Si el elemento es menor al otro se asigna a o b)
		arbol[inode].c1 = arbol[_a].cl > arbol[_b].cl ? _a : _b;//nodo interno 1 (Si el elemento es menor al otro se asigna a o b)
		arbol[inode].w = arbol[_a].w + arbol[_b].w;//asignación del peso
		arbol[inode].cl = arbol[_a].cl + 1;

		nodoLibre[a] = inode++;// Remplaza un nodo libre con uno recientemente creado
		nodoLibre[b] = nodoLibre[lfnode--];// Remplaza un nodo libre b con el último nodo libre;
	} 
   // Finalización de construcción del árbol

	/// Decodificación
	unsigned char mask = 0x80;//constante hexadecimal (128)
	unsigned char byte = fgetc(ent); //asignación del caracter en el flujo de entrada
	for(i = 0; i < length; i++){
		if(feof(sal)){
			printf("No se puede decodificar el archivo\n");
			exit(-1);
		}
		unsigned short curr = inode - 1;
		while(arbol[curr].cl){
			curr = byte & mask ? arbol[curr].c1 : arbol[curr].c0; //Si se cumple byte y mask se asigna c1 o c0
			mask >>= 1; //Desplazamiento de bit a la derecha
			if(!mask){
				mask = 0x80;
				byte = fgetc(ent);//obtención del caracter del flujo de entrada
			}
		}
		fputc(redirTab[curr], sal);//Escritura de los caracteres en el flujo de salida
	}
	//******************************************************************
	//Evaluar los tiempos de ejecucion
	//******************************************************************
    uswtime(&utime1, &stime1, &wtime1);
	//Calculo del tiempo de ejecucion del programa
    printf("real (Tiempo total)  %.10f s\n", wtime1 - wtime0);
    printf("user (Tiempo de procesamiento en CPU) %.10f s\n", utime1 - utime0);
    printf("sys (Tiempo en acciónes de E/S)  %.10f s\n", stime1 - stime0);
    printf("CPU/Wall   %.10f %% \n", 100.0 * (utime1 - utime0 + stime1 - stime0) / (wtime1 - wtime0));
    printf("\n");
    
    //Mostrar los tiempos en formato exponecial
    printf("\n");
    printf("real (Tiempo total)  %.10e s\n", wtime1 - wtime0);
    printf("user (Tiempo de procesamiento en CPU) %.10e s\n", utime1 - utime0);
    printf("sys (Tiempo en acciónes de E/S)  %.10e s\n", stime1 - stime0);
    printf("CPU/Wall   %.10f %% \n", 100.0 * (utime1 - utime0 + stime1 - stime0) / (wtime1 - wtime0));
    printf("\n");
}