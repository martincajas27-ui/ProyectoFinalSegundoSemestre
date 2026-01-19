#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ========== CONSTANTES ==========
#define MAX_REGISTROS 200000
#define MAX_ZONAS 5
#define MAX_BUFFER 1024
#define DIAS_HISTORICO 30
#define ZONAS_NOMBRES_CANTIDAD 5

// Límites OMS (en µg/m³ o ppm según sea el caso)
#define LIMITE_NO2_HORA 200.0f  // g/m promedio horario
#define LIMITE_NO2_ANUAL 40.0f  // g/m promedio anual
#define LIMITE_PM25_24H 35.0f   // g/m promedio 24h
#define LIMITE_PM25_ANUAL 15.0f // g/m promedio anual
#define LIMITE_SO2_HORA 350.0f  // g/m promedio horario
#define LIMITE_SO2_ANUAL 20.0f  // g/m promedio anual
#define LIMITE_O3_8H 100.0f     // g/m promedio 8h
#define LIMITE_CO_8H 10.0f      // mg/m promedio 8h

// ========== ESTRUCTURAS ==========

// Estructura para almacenar un registro de contaminación
typedef struct
{
    char fecha[20]; // Formato: YYYY-MM-DD
    char hora[10];  // Formato: HH:00:00
    float CO;
    float DIR; // Dirección del viento
    float HUM; // Humedad
    float LLU; // Lluvia
    float NO2;
    float O3;
    float PM25; // PM2.5
    float SO2;
    float TMP; // Temperatura
    float VEL; // Velocidad del viento
} RegistroContaminacion;

// Estructura para almacenar datos de una zona
typedef struct
{
    char nombre[50];
    RegistroContaminacion registros[MAX_REGISTROS];
    int cantidad_registros;
} DatosZona;

// Estructura para predicción de contaminación
typedef struct
{
    char fecha_prediccion[20];
    float prediccion_NO2;
    float prediccion_PM25;
    float prediccion_SO2;
    float prediccion_O3;
    float prediccion_CO;
    int alerta_nivel; // 1: Normal, 2: Alerta, 3: Crítica
} PrediccionContaminacion;

// Estructura para cálculos de índices
typedef struct
{
    float promedio_NO2;
    float promedio_PM25;
    float promedio_SO2;
    float promedio_O3;
    float promedio_CO;
    int estado_calidad; // 1: Buena, 2: Moderada, 3: Dañina, 4: Muy Dañina
} IndiceCalidad;

// ========== FUNCIONES DE CARGA DE DATOS ==========

// Carga todos los archivos .data de las zonas
DatosZona *cargar_datos_zonas(void);

// Carga un archivo .data específico de una zona
int cargar_archivo_zona(const char *ruta, DatosZona *zona);

// Obtiene el nombre de la zona basado en su índice
const char *obtener_nombre_zona(int indice);

// ========== FUNCIONES DE MONITOREO ACTUAL ==========

// Calcula el promedio de contaminación actual de los últimos 7 días
IndiceCalidad calcular_indice_actual(DatosZona *zona);

// Calcula el promedio histórico de los últimos 30 días
IndiceCalidad calcular_promedio_historico(DatosZona *zona);

// Verifica si un valor supera los límites OMS
int supera_limite_oms(int tipo, float valor);

// ========== FUNCIONES DE PREDICCIÓN ==========

// Predice los niveles de contaminación para las próximas 24 horas
PrediccionContaminacion predecir_contaminacion_24h(DatosZona *zona);

// Calcula el nivel de alerta basado en predicciones
int evaluar_nivel_alerta(PrediccionContaminacion *prediccion);

// ========== FUNCIONES DE RECOMENDACIONES ==========

// Genera recomendaciones basadas en niveles de contaminación
int generar_recomendaciones(DatosZona *zona, IndiceCalidad *indice,
                            PrediccionContaminacion *prediccion,
                            char recomendaciones[10][200],
                            int max_recomendaciones);

// ========== FUNCIONES DE EXPORTACIÓN ==========

// Exporta un reporte completo de todas las zonas
int exportar_reporte(DatosZona *zonas, int cantidad_zonas,
                     const char *nombre_archivo);

// Exporta el reporte de una zona específica
int exportar_reporte_zona(DatosZona *zona, IndiceCalidad *indice,
                          PrediccionContaminacion *prediccion,
                          char recomendaciones[10][200],
                          int cantidad_recomendaciones,
                          const char *nombre_archivo);

// ========== FUNCIONES DE VALIDACIÓN ==========

// Valida que una entrada de usuario sea numérica
int validar_entrada_numerica(char *entrada, int min, int max);

// Valida entrada de valores flotantes y rechaza letras y valores negativos donde no aplique
float validar_entrada_float_simple(const char *buffer, float min);

// Limpia el buffer de entrada
void limpiar_buffer(void);

// ========== FUNCIONES DE INTERFAZ ==========

// Muestra el menú principal
int mostrar_menu_principal(void);

// Muestra el menú de selección de zona
int mostrar_menu_zonas(void);

// Muestra el menú de análisis de una zona
int mostrar_menu_zona_analisis(void);

// Imprime los índices de calidad de forma formateada
void imprimir_indice_calidad(IndiceCalidad *indice, int tipo);

// Imprime la predicción de forma formateada
void imprimir_prediccion(PrediccionContaminacion *prediccion);

// Imprime las recomendaciones
void imprimir_recomendaciones(char recomendaciones[10][200], int cantidad);

// Imprime el estado de alertas para todas las zonas
void imprimir_estado_alertas(DatosZona *zonas, int cantidad_zonas);

// Lee un archivo de reporte de contaminación
int leer_reporte_contaminacion(const char *ruta);

// ========== FUNCIONES DE INGRESO DE DATOS ==========

// Permite ingresar nuevos datos del día para una zona específica
int ingresar_datos_dia(DatosZona *zonas, int cantidad_zonas);

#endif // FUNCIONES_H
