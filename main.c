#include "funciones.h"

int main(void)
{
    printf("\n");
    printf("==============================================================\n");
    printf("    Iniciando Sistema de Monitoreo de Contaminacion\n");
    printf("==============================================================\n");
    printf("\nCargando datos de las zonas...\n");

    DatosZona *zonas = cargar_datos_zonas();
    if (zonas == NULL)
    {
        printf("\nERROR CRITICO: No se pudieron cargar los datos\n");
        return 1;
    }

    printf("\n[OK] Sistema listo para operar\n");

    int opcion_principal = 0;
    int ejecutar = 1;

    while (ejecutar)
    {
        opcion_principal = mostrar_menu_principal();

        switch (opcion_principal)
        {
        case 1:
        {
            // Analizar una zona
            int zona_indice = mostrar_menu_zonas();

            if (zona_indice == -1)
            {
                continue;
            }

            DatosZona *zona_seleccionada = &zonas[zona_indice];
            int analisis_activo = 1;

            while (analisis_activo)
            {
                printf("\n");
                printf("=============================================\n");
                printf("ZONA: %s\n", zona_seleccionada->nombre);
                printf("Registros disponibles: %d\n", zona_seleccionada->cantidad_registros);
                printf("=============================================\n");

                int opcion_analisis = mostrar_menu_zona_analisis();

                switch (opcion_analisis)
                {
                case 1:
                {
                    // Ver índice actual
                    IndiceCalidad indice = calcular_indice_actual(zona_seleccionada);
                    imprimir_indice_calidad(&indice, 0);
                    break;
                }
                case 2:
                {
                    // Ver predicción 24h
                    PrediccionContaminacion prediccion = predecir_contaminacion_24h(zona_seleccionada);
                    imprimir_prediccion(&prediccion);
                    break;
                }
                case 3:
                {
                    // Ver promedio histórico
                    IndiceCalidad historico = calcular_promedio_historico(zona_seleccionada);
                    imprimir_indice_calidad(&historico, 1);
                    break;
                }
                case 4:
                {
                    // Ver recomendaciones
                    IndiceCalidad indice = calcular_indice_actual(zona_seleccionada);
                    PrediccionContaminacion prediccion = predecir_contaminacion_24h(zona_seleccionada);

                    char recomendaciones[10][200];
                    int cantidad_recom = generar_recomendaciones(
                        zona_seleccionada, &indice, &prediccion,
                        recomendaciones, 10);

                    imprimir_recomendaciones(recomendaciones, cantidad_recom);
                    break;
                }
                case 5:
                {
                    // Volver
                    analisis_activo = 0;
                    break;
                }
                case -1:
                {
                    // Error en entrada
                    printf("ERROR: Opcion invalida. Intente de nuevo.\n");
                    break;
                }
                default:
                {
                    printf("ERROR: Opcion no valida.\n");
                    break;
                }
                }
            }
            break;
        }

        case 2:
        {
            // Ver estado de alertas
            imprimir_estado_alertas(zonas, MAX_ZONAS);
            break;
        }

        case 3:
        {
            // Generar reporte completo
            printf("\nGenerando reporte completo...\n");

            char nombre_archivo[100];
            time_t ahora = time(NULL);
            struct tm *info = localtime(&ahora);
            snprintf(nombre_archivo, sizeof(nombre_archivo),
                     "Reporte_Contaminacion_%04d%02d%02d_%02d%02d%02d.txt",
                     info->tm_year + 1900,
                     info->tm_mon + 1,
                     info->tm_mday,
                     info->tm_hour,
                     info->tm_min,
                     info->tm_sec);

            if (exportar_reporte(zonas, MAX_ZONAS, nombre_archivo))
            {
                printf("[OK] Reporte generado exitosamente: %s\n", nombre_archivo);
            }
            else
            {
                printf("ERROR: No se pudo generar el reporte\n");
            }
            break;
        }

        case 4:
        {
            // Ingresar datos del día
            ingresar_datos_dia(zonas, MAX_ZONAS);
            break;
        }

        case 5:
        {
            // Salir
            printf("\n");
            printf("==============================================================\n");
            printf("           Gracias por usar el sistema\n");
            printf("==============================================================\n");
            printf("\n");
            ejecutar = 0;
            break;
        }

        case -1:
        {
            // Error en entrada
            printf("ERROR: Opcion invalida. Intente de nuevo.\n");
            break;
        }

        default:
        {
            printf("ERROR: Opcion no valida.\n");
            break;
        }
        }
    }

    // Liberar memoria
    free(zonas);

    return 0;
}
