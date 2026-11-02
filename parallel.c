#define _GNU_SOURCE
#include <stdio.h>  // file handling functions
#include <stdlib.h> // atoi
#include <string.h> // strtok
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>
#define stacksize 1048576

double **data;
int data_nrows;
int data_ncols = 784;
char *my_path = "/users/alumnos/acaf/acaf0319/archivosParallel";  // vosotros tendréis que poner vuestro path

int seed = 0;
int matrices_rows[4] = {784, 200, 100, 50};
int matrices_columns[4] = {200, 100, 50, 10};
int vector_rows[4] = {200, 100, 50, 10};
char *str;

int rows_per_div;
static double *digits;
static double **mat1;
static double **mat2;
static double **mat3;
static double **mat4;
static double *vec1;
static double *vec2;
static double *vec3;
static double *vec4;

int read_matrix(double** mat, char* file, int nrows, int ncols, int fac){
    /*
     * Dada una matrix (mat), un nombre de fichero (file), una cantidad de filas (nrows) y columnas (ncols), y un multiplicador (no se usa),
     * deja en mat la matriz (de dimensión nrows x ncols) de datos contenida en el fichero con nombre file
     */

    char *buffer = malloc(2048);
    char *record;
    FILE *fstream = fopen(file,"r");
    double aux;
    if(fstream == NULL)
    {
        fprintf(stderr, "\n file opening failed %s ", file);
        return -1 ;
    }
    for(int row=0; row<nrows; row++)
    {
        fgets(buffer,2048,fstream);
        record = strtok(buffer," ");
        mat[row] = malloc(ncols * sizeof(double));
        for(int column=0;column<ncols;column++)
        {
            if(record){
                aux = strtod(record, NULL)*(float)fac;
                mat[row][column] = aux;
            }
            else {
                mat[row][column] = -1.0;
            }
            record = strtok(NULL," ");
        }
    }
    fclose(fstream);
    free(buffer);
    return 0;
}

int read_vector(double* vect, char* file, int nrows) {
    /*
     * Dado un vector (vect), un nombre de fichero (file), y una cantidad de filas (nrows),
     * deja en vect el vector (de dimensión nrows) de datos contenido en el fichero con nombre file
     */
    char *buffer = malloc(2048);
    FILE *fstream = fopen(file, "r");
    double aux;
    if (fstream == NULL) {
        fprintf(stderr, "\n file opening failed %s ", file);
        return -1;
    }
    for (int row = 0; row < nrows; row++) {
        fgets(buffer, 2048, fstream);

        aux = strtod(buffer, NULL);
        vect[row] = aux;
    }
    fclose(fstream);
    free(buffer);
    return 0;
}

int relu(double** mat, int nrows, int ncols){
    /*
     * Dada una matriz (mat) y sus dimensiones, aplica la función ReLU a todos sus elementos
     */
    for(int row=0;row<nrows;row++){
        for(int col=0;col<ncols;col++){
            if(mat[row][col]<0)
                mat[row][col] = 0;
        }
    }
    return 0;
}

int print_matrix(double** mat, int nrows, int ncols, int offset_row, int offset_col){
    /*
     * Dada una matriz (mat), una cantidad de filas (nrows) y columnas (ncols) a imprimir, y una cantidad de filas (offset_row)
     * y columnas (offset_col) a ignorar, imprime por salida estándar nrows x ncols de la matriz
     */
    for(int row=0;row<nrows;row++){
        for(int col=0;col<ncols;col++){
            printf("%f ", mat[row+offset_row][col+offset_col]);
        }
        printf("\n");
    }
    return 0;
}

int matmul(double** matr1, double** matr2, double** res, int rows1_start, int rows1_end, int ncols1, int ncols2) {
    /*
     * Dadas dos matrices (mat1 y mat2), una matriz resultado (res), dos índices de filas de la primera matriz (rows1_start y rows_end),
     * y la cantidad de columnas de mat1 y mat2, calcula el producto matricial entre las filas contenidas entre rows1_start y rows1_end
     * de mat1, y mat2 (todas sus columnas).
     */
    double aux, a1, a2;
    for (int row = 0; row < rows1_end - rows1_start; row++) {
        res[row] = malloc(ncols2 * sizeof(double));
        for (int col2 = 0; col2 < ncols2; col2++) {
            aux = 0;
            for (int col1_row2 = 0; col1_row2 < ncols1; col1_row2++) {
                a1 = matr1[row + rows1_start][col1_row2];
                a2 = matr2[col1_row2][col2];
                aux += (a1 * a2);
            }
            res[row][col2] = aux;
        }
    }
    return 0;
}

int argmax(double** mat, int nrows, int ncols, int* predictions){
    /*
     * Dada una matriz de valores (mat) y sus dimensiones (nrows y ncols), y un vector vacío (predictions), esta función
     * calculará, para cada una de las filas de la matriz mat, la posición del valor más alto, y lo colocará en predictions
     */
    double max;
    int argmax;
    for(int row=0;row<nrows;row++){
        max = -9999999;
        argmax = -1;
        for(int col=0;col<ncols;col++){
            if(mat[row][col]>max) {
                max = mat[row][col];
                argmax = col;
            }
        }
        predictions[row] = argmax;
    }
    return 0;
}

int add_vector(double** mat, const double* vect, int nrows, int ncols){
    /*
     * Dada una matriz (mat), un vector (vect), la cantidad de filas de mat y vect, y de columnas de mat, suma los valores
     * de vect a cada columna de mat
     */
    for(int row=0;row<nrows;row++){
        for(int col=0;col<ncols;col++){
            mat[row][col] += vect[col];
        }
    }
    return 0;
}

int load_data(char* path){


    /*
     * Dado un directorio en el que están los datos y parámetros, los carga en las variables de entorno
     */

    digits = malloc(data_nrows*sizeof(double));  // Los valores que idealmente predeciremos
    sprintf(str, "%s/digits.csv", path);
    read_vector(digits, str, data_nrows);

    mat1 = malloc(matrices_rows[0] * sizeof(double *));//Asignamos la memoria de la mat1, vec1....
    sprintf(str, "%s/weights0_19.csv", path);
    read_matrix(mat1, str, matrices_rows[0], matrices_columns[0], 1);
    vec1 = malloc(vector_rows[0] * sizeof(double));
    sprintf(str, "%s/biases0_19.csv", path);
    read_vector(vec1, str, vector_rows[0]);

    mat2 = malloc(matrices_rows[1] * sizeof(double *));
    sprintf(str, "%s/weights1_19.csv", path);
    read_matrix(mat2, str, matrices_rows[1], matrices_columns[1], 1);
    vec2 = malloc(vector_rows[1] * sizeof(double));
    sprintf(str, "%s/biases1_19.csv", path);
    read_vector(vec2, str, vector_rows[1]);

    mat3 = malloc(matrices_rows[2] * sizeof(double *));
    sprintf(str, "%s/weights2_19.csv", path);
    read_matrix(mat3, str, matrices_rows[2], matrices_columns[2], 1);
    vec3 = malloc(vector_rows[2] * sizeof(double));
    sprintf(str, "%s/biases2_19.csv", path);
    read_vector(vec3, str, vector_rows[2]);

    mat4 = malloc(matrices_rows[3] * sizeof(double *));
    sprintf(str, "%s/weights3_19.csv", path);
    read_matrix(mat4, str, matrices_rows[3], matrices_columns[3], 1);
    vec4 = malloc(vector_rows[3] * sizeof(double));
    sprintf(str, "%s/biases3_19.csv", path);
    read_vector(vec4, str, vector_rows[3]);

    //Por ultimo cargamos la matriz de datos(data.csv) 
    //La inicializamos de manera diferente ya que sino nos daba error
    data = malloc(data_nrows * sizeof(double *));
    for (int i = 0; i < data_nrows; ++i) {
    data[i] = malloc(data_ncols * sizeof(double));
    }
    sprintf(str, "%s/data.csv", path);
    read_matrix(data, str, data_nrows, data_ncols, 1);

    return 0;
}


int unload_data(){
    // Liberamos toda la memoria almacenada
    free(digits);
    free(data);

    //Memoria de las matrices y vectores de la capa 0
    for (int i = 0; i < matrices_rows[0]; ++i)
        free(mat1[i]);
    free(mat1);
    free(vec1);

    //Memoria de las matrices y vectores de la capa 1
    for (int i = 0; i < matrices_rows[1]; ++i)
        free(mat2[i]);
    free(mat2);
    free(vec2);

    //Memoria de las matrices y vectores de la capa 2
    for (int i = 0; i < matrices_rows[2]; ++i)
        free(mat3[i]);
    free(mat3);
    free(vec3);

    //Memoria de las matrices y vectores de la capa 3
    for (int i = 0; i < matrices_rows[3]; ++i)
        free(mat4[i]);
    free(mat4);
    free(vec4);

    return 0;
}


int perform_multiplications(void* arg) {
    //variables locales
    int aciertos = 0;
    int *predics = malloc(rows_per_div * sizeof(int));
    int start = *(int *)arg * rows_per_div;
    int end = start + rows_per_div;

    // Variables para las capas
    double **capa0, **capa1, **capa2, **capa3, **resultado;
    
    // Inicialización de matrices para capa0
    capa0 = malloc(rows_per_div * sizeof(double *));
    capa1 = malloc(rows_per_div * sizeof(double *));
    capa2 = malloc(rows_per_div * sizeof(double *));
    capa3 = malloc(rows_per_div * sizeof(double *));
    resultado = malloc(rows_per_div * sizeof(double *));
    for (int i = 0; i < rows_per_div; ++i) {
        capa0[i] = malloc(matrices_columns[0] * sizeof(double));
        capa1[i] = malloc(matrices_columns[1] * sizeof(double));
        capa2[i] = malloc(matrices_columns[2] * sizeof(double));
        capa3[i] = malloc(matrices_columns[3] * sizeof(double));
        resultado[i] = malloc(matrices_columns[3] * sizeof(double));
    }

    // Procesam las capas usando el siguiente patron(Basicamente lo que hay en el enunciado del trabajo)
    matmul(data, mat1, capa0, start, end, data_ncols, matrices_columns[0]);
    add_vector(capa0, vec1, rows_per_div, matrices_columns[0]);
    relu(capa0, rows_per_div, matrices_columns[0]);

    matmul(capa0, mat2, capa1, 0, rows_per_div, matrices_columns[0], matrices_columns[1]);
    add_vector(capa1, vec2, rows_per_div, matrices_columns[1]);
    relu(capa1, rows_per_div, matrices_columns[1]);

    matmul(capa1, mat3, capa2, 0, rows_per_div, matrices_columns[1], matrices_columns[2]);
    add_vector(capa2, vec3, rows_per_div, matrices_columns[2]);
    relu(capa2, rows_per_div, matrices_columns[2]);

    matmul(capa2, mat4, capa3, 0, rows_per_div, matrices_columns[2], matrices_columns[3]);
    add_vector(capa3, vec4, rows_per_div, matrices_columns[3]);
    relu(capa3, rows_per_div, matrices_columns[3]);

    
    relu(capa3, rows_per_div, matrices_columns[3]);

    
    argmax(capa3, rows_per_div, matrices_columns[3], predics);

    //Impresion de los resultados a un fichero del proceso actual
     char* archiPredicciones = malloc(sizeof(char) * (strlen(my_path) + 50));
    sprintf(archiPredicciones, "%s/predi%d", my_path, *(int *)arg);
    FILE *f = fopen(archiPredicciones, "w");
    if (f == NULL) {
        printf("Error al abrir el archivo de predicciones\n");
        return -1;
    }
    for(int i=0;i<rows_per_div; i++){
        fprintf(f, "%d \n", predics[i]);
    }
    fclose(f);
    




    // Liberamos la memoria usada
    for (int i = 0; i < rows_per_div; i++) {
        free(capa0[i]);
        free(capa1[i]);
        free(capa2[i]);
        free(capa3[i]);
        free(resultado[i]);
    }
    free(capa0);
    free(capa1);
    free(capa2);
    free(capa3);
    free(resultado);
    free(predics);
    free(archiPredicciones);

    return 0;
}

int main(int argc, char* argv[]){
    /*
     * El programa recibe un único argumento, la cantidad de procesos que se emplearán en la paralelización.
     * Por ejemplo, parallel 3 tendrá que dividir la matriz en tres, y lanzar tres procesos paralelos. Cada proceso, deberá
     * procesar un tercio de la matriz de datos
     */

    if (argc!=2){
        printf("El programa debe tener un único argumento\n");
        exit(1);
    }
    str = (char*) malloc(sizeof(char)*(strlen(my_path)+20));
    data_nrows = 60000;  // Cantidad de datos para multiplicar: 800 para ver si va bien, 60.000 para la prueba del tiempo
    load_data(my_path); //Cargamos las matrices del directorio en las variables de entorno
    int divisions = (int)strtol(argv[1], NULL, 10);
    int pids[divisions];
    char *stack[divisions];
    rows_per_div = data_nrows / divisions;
    
    for(int i=0; i<divisions; i++){
        stack[i] = malloc(stacksize); // Hacemos la reserva el stack de este proceso
        pids[i] = clone((int (*)(void *)) &perform_multiplications, stack[i] + stacksize, SIGCHLD, (void *)(&i)); 
    }
   
    sprintf(str, "%s/predicciones.txt", my_path);//Camino para los ficheros que llenaran los procesos hijo
    FILE* resul = fopen(str, "w+");
    unsigned char buffer[4096];//Tamaño que usabamos en codigos de trabajos anteriores y que usaremos en este caso para pasar el cocntenido de los procesos hijo a predicciones.txt
                               /*Esta informacion la vimos en un foro de StackOverflow, de un usuario que decia lo siguiente:
                               "size_t is a type that can hold any array index.
                                Depending on the implementation, it can be any of:
                                -unsigned char ...*/
    for(int i=0; i<divisions; i++){
        waitpid(pids[i], NULL, 0);

        sprintf(str, "%s/predi%d", my_path, i);
        FILE* read = fopen(str, "r");
        size_t NumerosHijo = fread(buffer, sizeof(char), sizeof(buffer), read);
        while(NumerosHijo!=0){
            fwrite(buffer, sizeof(char), NumerosHijo, resul);
            NumerosHijo = fread(buffer, sizeof(char), sizeof(buffer), read);
        }
        fclose(read);
    }
    fclose(resul);
       
    
       //Para saber que tan precisa sera nuestra prediccion,vamos a hacer una comparación de predicciones con el vector digits y los 
       //numeros obtenidos en predicciones.txt durante la ejecucion de los procesos de perform_multiplications y
       // haremos un calculo para saber cual ha sido la precisión obtenida
    int aciertos_totales = 0;
    FILE *prediccion = fopen("predicciones.txt", "r");
    if (prediccion == NULL) {
        printf("Error al abrir el archivo de predicciones\n");
        exit(1);
    }
    int pred, digit;
    for (int i = 0; i < data_nrows; i++) {
        fscanf(prediccion, "%d", &pred);
        digit = (int)digits[i];
        if (pred == digit) {
            aciertos_totales++;
        }
    }
    fclose(prediccion);

    // Hacemos el calculo de la precisión
    float precision = (float)aciertos_totales / data_nrows;
    printf("Precision obtenida: %.2f%%\n", precision * 100);//Imprimimos la precision
    // Liberar memoria y finalizar programa
    unload_data();
    free(str);
    return 0;
}
