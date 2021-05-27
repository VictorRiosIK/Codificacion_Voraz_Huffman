#include <stdio.h>
#include <stdlib.h>

//ent: flujo de datos a comprimir
//sal: flujo de datos comprimido
void huffman_coder(FILE * ent, FILE * sal);

struct node
{
	unsigned int w; // Peso
	unsigned short p; // Padre
	unsigned short c0, c1; // 0 / 1
	unsigned char cl; // Tamaño del código
};

int main(int argv, char** argc)
{
	if(argv != 4)
	{
		printf("Uso: %s [c/d] input output\n", argc[0]);
		return 0;
	}

	FILE * ent, * sal;

	if(!(ent = fopen(argc[2], "rb")))
	{
		perror("ERRROR opening input file");
		return -1;
	}
	if(!(sal = fopen(argc[3], "wb")))
	{
		perror("ERROR opening output file");
		return -1;
	}

	if(*argc[1] == 'c')
		huffman_coder(ent, sal);
	else
		printf("Uso: %s [c/d] input output\n", argc[0]);

	fclose(ent);
	fclose(sal);
	return 0;
}

void huffman_coder(FILE * ent, FILE * sal){
	double utime0, stime0, wtime0, utime1, stime1, wtime1; //Variables para medicion de tiempos
	//******************************************************************
	//Iniciar el conteo del tiempo para las evaluaciones de rendimiento
	//******************************************************************
    uswtime(&utime0, &stime0, &wtime0);
    
	int i, j;

	//Tabla de frecuencias
	unsigned int freqTable[256] = {0}; // Contador para frecuencias por palabra.
	unsigned char redirTable[256]; // Tabla de redirección.

	while(!feof(ent)) //Iterar por el flujo de entrada
		freqTable[fgetc(ent)]++; // Incrementar el contador para el caracter.

	/// Construcción del árbol
	struct node arbol[512000] = {0}; // Memoria para alojar el árbol
	unsigned short nodoLibre[256]; // Lista de sub arboles libres
	unsigned short a, b; // a = nodo más pequeño; b = segundo nodo más pequeño

	j = 0;
	for(i = 0; i < 256; i++){ // Inicializa hojas y libera nodos
		if(freqTable[i]){
			redirTable[i] = j;
			nodoLibre[j] = j;
			arbol[j].w = freqTable[i];
			j++;
		}
	}
	unsigned short inode = j; // Siguiente nodo libre
	unsigned short lfnode = j - 1; // última posición ocupada en nodosLibres

	while(lfnode > 0){ // Iterar hasta que el árbol este completo
		if(arbol[nodoLibre[0]].w < arbol[nodoLibre[1]].w){ // Inicializar a y b
			a = 0;
			b = 1;
		}else{
			b = 0;
			a = 1;
		}
		for(i = 2; i <= lfnode; i++){ // Encontrar los 2 nodos más pequeños
			unsigned int w = arbol[nodoLibre[i]].w;
			if(w < arbol[nodoLibre[a]].w){
				b = a;
				a = i;
			}else{
				if(w < arbol[nodoLibre[b]].w)
				b = i;
			}
		}
		// Llenar datos
		unsigned short _a = nodoLibre[a];
		unsigned short _b = nodoLibre[b];
		arbol[_a].p = inode;
		arbol[_b].p = inode;
		arbol[inode].c0 = arbol[_a].cl <= arbol[_b].cl ? _a : _b;
		arbol[inode].c1 = arbol[_a].cl > arbol[_b].cl ? _a : _b;
		arbol[inode].w = arbol[_a].w + arbol[_b].w;
		arbol[inode].cl = arbol[_a].cl + 1;

		nodoLibre[a] = inode++; // Remplaza un nodo libre con uno recientemente creado
		nodoLibre[b] = nodoLibre[lfnode--]; // Remplaza un nodo libre b con el último nodo libre;

	} // Finalización de construcción del árbol


	/// Codificar el flujo
	unsigned char stack[256]; // Pila de nodos atravesados
	unsigned char stackp = 0; // Apuntador a pila
	size_t fs = ftell(ent); // Tamaño del archivo

	fwrite(&fs, sizeof(size_t), 1, sal); // Escribir tamaño de archivo sin comprimir
	fwrite(freqTable, sizeof(unsigned int), 256, sal); // Escribir tabla de frecuencias
	rewind(ent); // De regreso al inicio.

	unsigned char mask = 0x80;
	unsigned char byte = 0x00;
	while(1) // Codificar todos los bytes.
	{
		unsigned short inraw = fgetc(ent);
		if(feof(ent)) break;
		unsigned short curr = redirTable[inraw];

		while(arbol[curr].p) // IIterar de regreso por el arbol
		{
			stack[stackp++] = (curr == arbol[arbol[curr].p].c1) ? 0xFF : 0x00; // Agregar bit al apuntador.
			curr = arbol[curr].p;
		}

		while(stackp)
		{
			byte |= mask & stack[--stackp];
			mask >>= 1;
			if(!mask)
			{
				mask = 0x80;
				fputc(byte, sal);
				byte = 0x00;
			}
		}
	}
	if(mask != 0x80) fputc(byte, sal);
	
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
