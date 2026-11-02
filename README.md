# Red Neuronal Paralela en C (`parallel.c`)

Este repositorio contiene una implementación en **C** de una **red neuronal multicapa de propagación hacia adelante (feedforward neural network)** paralelizada mediante la llamada al sistema `clone()` de Linux.

El programa está diseñado para procesar y clasificar conjuntos de datos masivos dividiendo el trabajo de forma eficiente entre múltiples procesos concurrentes.

---

## 🚀 Características Principales

- **Programación de Sistemas en C:** Gestión dinámica de memoria, matrices complejas y procesamiento de archivos.
- **Paralelización con `clone()`:** Creación de procesos ligeros a bajo nivel con pilas (`stack`) independientes y sincronización con `waitpid()`.
- **Inferencia de Red Neuronal Profunda:** 
  - Carga de pesos (`weights`) y sesgos (`biases`) distribuidos en 4 capas.
  - Operaciones de producto matricial (`matmul`), activación **ReLU** (`relu`) y clasificación final (`argmax`).
- **Evaluación de Rendimiento:** Agregación de resultados parciales y cálculo automático del porcentaje de precisión comparando con `digits.csv`.

---

## 📁 Ficheros del Repositorio

- `parallel.c`: Código fuente principal con la lógica de paralelización e inferencia.
- `weights*.csv` / `biases*.csv`: Matrices de pesos y vectores de sesgos para las 4 capas de la red.
- `data.csv`: Matriz de datos de entrada a clasificar.
- `digits.csv`: Vector con las etiquetas reales para calcular la precisión final.

---

## ⚙️ Compilación y Ejecución

### 📋 Requisitos Previos

- **Sistema Operativo:** Entorno Linux o WSL (Windows Subsystem for Linux), ya que el código utiliza funciones nativas y directivas específicas como `_GNU_SOURCE` y la syscall `clone()`.
- **Compilador:** `gcc` (GNU Compiler Collection).
- **Ficheros de entrada:** Asegúrate de tener en el mismo directorio (o en la ruta especificada) los archivos `.csv` necesarios (`data.csv`, `digits.csv`, `weights*.csv` y `biases*.csv`).

### ⚠️ Configuración Previa de Rutas

Antes de compilar, revisa la variable `my_path` en la línea 10 de `parallel.c`:

```c
char *my_path = "."; // Cambia a "." si los archivos .csv están en la misma carpeta que el ejecutable
```

### 🛠️ Compilación

Abre tu terminal en el directorio del proyecto y ejecuta el siguiente comando:

```bash
gcc -o parallel parallel.c -Wall
```

### 🚀 Ejecución

El programa **requiere un parámetro obligatorio**: la cantidad de procesos paralelos en los que se dividirá el conjunto de datos de 60,000 elementos.

#### Ejemplo de ejecución con 4 procesos:

```bash
./parallel 4
```

#### Ejemplo de salida esperada:

```text
Precision obtenida: 95.50%
```