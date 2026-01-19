#include "funciones.h"

// ========== VARIABLES GLOBALES ==========
const char *NOMBRES_ZONAS[ZONAS_NOMBRES_CANTIDAD] = {
    "Belisario",
    "Carapungo",
    "Centro",
    "Cotocollao",
    "Elcamal"};

const char *ARCHIVOS_DATOS[ZONAS_NOMBRES_CANTIDAD] = {
    "Belisario_contaminacion.data",
    "Carapungo_contaminacion.data",
    "Centro_contaminacion.data",
    "Cotocollao_contaminacion.data",
    "Elcamal_contaminacion.data"};

// ========== FUNCIONES DE CARGA DE DATOS ==========

const char *obtener_nombre_zona(int indice)
{
    if (indice >= 0 && indice < ZONAS_NOMBRES_CANTIDAD)
    {
        return NOMBRES_ZONAS[indice];
    }
    return "Desconocida";
}

int cargar_archivo_zona(const char *ruta, DatosZona *zona)
{
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL)
    {
        printf("ERROR: No se puede abrir el archivo: %s\n", ruta);
        return 0;
    }

    char buffer[MAX_BUFFER];
    int cantidad = 0;

    // Saltar líneas de encabezado
    while (fgets(buffer, sizeof(buffer), archivo) != NULL)
    {
        if (buffer[0] != '#' && strlen(buffer) > 5)
        {
            break;
        }
    }

    // Procesar primer registro (ya fue leído en el while anterior)
    if (sscanf(buffer, "%s %s %f %f %f %f %f %f %f %f %f %f",
               zona->registros[cantidad].fecha,
               zona->registros[cantidad].hora,
               &zona->registros[cantidad].CO,
               &zona->registros[cantidad].DIR,
               &zona->registros[cantidad].HUM,
               &zona->registros[cantidad].LLU,
               &zona->registros[cantidad].NO2,
               &zona->registros[cantidad].O3,
               &zona->registros[cantidad].PM25,
               &zona->registros[cantidad].SO2,
               &zona->registros[cantidad].TMP,
               &zona->registros[cantidad].VEL) == 12)
    {
        cantidad++;
    }

    // Leer resto de registros
    while (fgets(buffer, sizeof(buffer), archivo) != NULL && cantidad < MAX_REGISTROS)
    {
        if (buffer[0] == '#' || strlen(buffer) <= 5)
        {
            continue;
        }

        if (sscanf(buffer, "%s %s %f %f %f %f %f %f %f %f %f %f",
                   zona->registros[cantidad].fecha,
                   zona->registros[cantidad].hora,
                   &zona->registros[cantidad].CO,
                   &zona->registros[cantidad].DIR,
                   &zona->registros[cantidad].HUM,
                   &zona->registros[cantidad].LLU,
                   &zona->registros[cantidad].NO2,
                   &zona->registros[cantidad].O3,
                   &zona->registros[cantidad].PM25,
                   &zona->registros[cantidad].SO2,
                   &zona->registros[cantidad].TMP,
                   &zona->registros[cantidad].VEL) == 12)
        {
            cantidad++;
        }
    }

    fclose(archivo);
    zona->cantidad_registros = cantidad;
    return 1;
}

DatosZona *cargar_datos_zonas(void)
{
    DatosZona *zonas = (DatosZona *)malloc(MAX_ZONAS * sizeof(DatosZona));
    if (zonas == NULL)
    {
        printf("ERROR: No se pudo asignar memoria para las zonas\n");
        return NULL;
    }

    for (int i = 0; i < MAX_ZONAS; i++)
    {
        strcpy(zonas[i].nombre, NOMBRES_ZONAS[i]);
        zonas[i].cantidad_registros = 0;

        if (!cargar_archivo_zona(ARCHIVOS_DATOS[i], &zonas[i]))
        {
            printf("ERROR: No se pudo cargar la zona %s\n", NOMBRES_ZONAS[i]);
            free(zonas);
            return NULL;
        }

        printf("[OK] Cargados %d registros de %s\n",
               zonas[i].cantidad_registros, NOMBRES_ZONAS[i]);
    }

    return zonas;
}

// ========== FUNCIONES DE MONITOREO ACTUAL ==========

int supera_limite_oms(int tipo, float valor)
{
    if (valor < 0)
        return 0; // Dato inválido

    switch (tipo)
    {
    case 0: // NO2
        return valor > LIMITE_NO2_HORA;
    case 1: // PM2.5
        return valor > LIMITE_PM25_24H;
    case 2: // SO2
        return valor > LIMITE_SO2_HORA;
    case 3: // O3
        return valor > LIMITE_O3_8H;
    case 4: // CO
        return valor > LIMITE_CO_8H;
    default:
        return 0;
    }
}

IndiceCalidad calcular_indice_actual(DatosZona *zona)
{
    IndiceCalidad indice;
    indice.promedio_NO2 = 0;
    indice.promedio_PM25 = 0;
    indice.promedio_SO2 = 0;
    indice.promedio_O3 = 0;
    indice.promedio_CO = 0;

    if (zona->cantidad_registros == 0)
    {
        indice.estado_calidad = 2;
        return indice;
    }

    // Tomar los últimos 7 días (aproximadamente 168 registros si hay dato cada hora)
    int inicio = zona->cantidad_registros - 168;
    if (inicio < 0)
        inicio = 0;

    int contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].NO2 > 0)
        {
            indice.promedio_NO2 += zona->registros[i].NO2;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_NO2 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].PM25 > 0)
        {
            indice.promedio_PM25 += zona->registros[i].PM25;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_PM25 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].SO2 > 0)
        {
            indice.promedio_SO2 += zona->registros[i].SO2;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_SO2 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].O3 > 0)
        {
            indice.promedio_O3 += zona->registros[i].O3;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_O3 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].CO > 0)
        {
            indice.promedio_CO += zona->registros[i].CO;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_CO /= contador;

    // Determinar estado de calidad basado en PM2.5 (indicador principal)
    if (indice.promedio_PM25 <= 12)
    {
        indice.estado_calidad = 1; // Buena
    }
    else if (indice.promedio_PM25 <= 35.5)
    {
        indice.estado_calidad = 2; // Moderada
    }
    else if (indice.promedio_PM25 <= 55.5)
    {
        indice.estado_calidad = 3; // Dañina
    }
    else
    {
        indice.estado_calidad = 4; // Muy Dañina
    }

    return indice;
}

IndiceCalidad calcular_promedio_historico(DatosZona *zona)
{
    IndiceCalidad indice;
    indice.promedio_NO2 = 0;
    indice.promedio_PM25 = 0;
    indice.promedio_SO2 = 0;
    indice.promedio_O3 = 0;
    indice.promedio_CO = 0;

    if (zona->cantidad_registros == 0)
    {
        indice.estado_calidad = 2;
        return indice;
    }

    // Últimos 30 días (aproximadamente 720 registros si hay dato cada hora)
    int inicio = zona->cantidad_registros - 720;
    if (inicio < 0)
        inicio = 0;

    int contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].NO2 > 0)
        {
            indice.promedio_NO2 += zona->registros[i].NO2;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_NO2 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].PM25 > 0)
        {
            indice.promedio_PM25 += zona->registros[i].PM25;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_PM25 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].SO2 > 0)
        {
            indice.promedio_SO2 += zona->registros[i].SO2;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_SO2 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].O3 > 0)
        {
            indice.promedio_O3 += zona->registros[i].O3;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_O3 /= contador;

    contador = 0;
    for (int i = inicio; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].CO > 0)
        {
            indice.promedio_CO += zona->registros[i].CO;
            contador++;
        }
    }
    if (contador > 0)
        indice.promedio_CO /= contador;

    // Determinar estado de calidad
    if (indice.promedio_PM25 <= 12)
    {
        indice.estado_calidad = 1;
    }
    else if (indice.promedio_PM25 <= 35.5)
    {
        indice.estado_calidad = 2;
    }
    else if (indice.promedio_PM25 <= 55.5)
    {
        indice.estado_calidad = 3;
    }
    else
    {
        indice.estado_calidad = 4;
    }

    return indice;
}

// ========== FUNCIONES DE PREDICCIÓN ==========

PrediccionContaminacion predecir_contaminacion_24h(DatosZona *zona)
{
    PrediccionContaminacion prediccion;
    strcpy(prediccion.fecha_prediccion, "Próximas 24 horas");

    if (zona->cantidad_registros < 10)
    {
        prediccion.prediccion_NO2 = -1;
        prediccion.prediccion_PM25 = -1;
        prediccion.prediccion_SO2 = -1;
        prediccion.prediccion_O3 = -1;
        prediccion.prediccion_CO = -1;
        prediccion.alerta_nivel = 1;
        return prediccion;
    }

    // Usar promedio ponderado de los últimos 30 días
    // Pesos: últimos 7 días (50%), 7-14 días (30%), 14-30 días (20%)
    float suma_NO2 = 0, suma_PM25 = 0, suma_SO2 = 0, suma_O3 = 0, suma_CO = 0;
    float peso_total = 0;

    int inicio = zona->cantidad_registros - 720; // 30 días
    if (inicio < 0)
        inicio = 0;

    // Últimos 7 días (peso 0.5)
    int fin_semana = zona->cantidad_registros - 168;
    if (fin_semana < inicio)
        fin_semana = inicio;

    for (int i = fin_semana; i < zona->cantidad_registros; i++)
    {
        if (zona->registros[i].NO2 > 0)
            suma_NO2 += zona->registros[i].NO2 * 0.5f;
        if (zona->registros[i].PM25 > 0)
            suma_PM25 += zona->registros[i].PM25 * 0.5f;
        if (zona->registros[i].SO2 > 0)
            suma_SO2 += zona->registros[i].SO2 * 0.5f;
        if (zona->registros[i].O3 > 0)
            suma_O3 += zona->registros[i].O3 * 0.5f;
        if (zona->registros[i].CO > 0)
            suma_CO += zona->registros[i].CO * 0.5f;
        peso_total += 0.5f;
    }

    // 7-14 días (peso 0.3)
    int fin_2semana = fin_semana - 168;
    if (fin_2semana < inicio)
        fin_2semana = inicio;

    float peso_temp = 0;
    for (int i = fin_2semana; i < fin_semana; i++)
    {
        if (zona->registros[i].NO2 > 0)
            suma_NO2 += zona->registros[i].NO2 * 0.3f;
        if (zona->registros[i].PM25 > 0)
            suma_PM25 += zona->registros[i].PM25 * 0.3f;
        if (zona->registros[i].SO2 > 0)
            suma_SO2 += zona->registros[i].SO2 * 0.3f;
        if (zona->registros[i].O3 > 0)
            suma_O3 += zona->registros[i].O3 * 0.3f;
        if (zona->registros[i].CO > 0)
            suma_CO += zona->registros[i].CO * 0.3f;
        peso_temp += 0.3f;
    }
    peso_total += peso_temp;

    // 14-30 días (peso 0.2)
    for (int i = inicio; i < fin_2semana; i++)
    {
        if (zona->registros[i].NO2 > 0)
            suma_NO2 += zona->registros[i].NO2 * 0.2f;
        if (zona->registros[i].PM25 > 0)
            suma_PM25 += zona->registros[i].PM25 * 0.2f;
        if (zona->registros[i].SO2 > 0)
            suma_SO2 += zona->registros[i].SO2 * 0.2f;
        if (zona->registros[i].O3 > 0)
            suma_O3 += zona->registros[i].O3 * 0.2f;
        if (zona->registros[i].CO > 0)
            suma_CO += zona->registros[i].CO * 0.2f;
        peso_temp += 0.2f;
    }
    peso_total += peso_temp;

    if (peso_total > 0)
    {
        prediccion.prediccion_NO2 = suma_NO2 / peso_total;
        prediccion.prediccion_PM25 = suma_PM25 / peso_total;
        prediccion.prediccion_SO2 = suma_SO2 / peso_total;
        prediccion.prediccion_O3 = suma_O3 / peso_total;
        prediccion.prediccion_CO = suma_CO / peso_total;
    }
    else
    {
        prediccion.prediccion_NO2 = -1;
        prediccion.prediccion_PM25 = -1;
        prediccion.prediccion_SO2 = -1;
        prediccion.prediccion_O3 = -1;
        prediccion.prediccion_CO = -1;
    }

    prediccion.alerta_nivel = evaluar_nivel_alerta(&prediccion);
    return prediccion;
}

int evaluar_nivel_alerta(PrediccionContaminacion *prediccion)
{
    int alertas_criticas = 0;
    int alertas_moderadas = 0;

    // Evaluar PM2.5 (más crítico)
    if (prediccion->prediccion_PM25 > LIMITE_PM25_24H * 1.5f)
    {
        alertas_criticas++;
    }
    else if (prediccion->prediccion_PM25 > LIMITE_PM25_24H)
    {
        alertas_moderadas++;
    }

    // Evaluar NO2
    if (prediccion->prediccion_NO2 > LIMITE_NO2_HORA * 1.5f)
    {
        alertas_criticas++;
    }
    else if (prediccion->prediccion_NO2 > LIMITE_NO2_HORA)
    {
        alertas_moderadas++;
    }

    // Evaluar SO2
    if (prediccion->prediccion_SO2 > LIMITE_SO2_HORA * 1.5f)
    {
        alertas_criticas++;
    }
    else if (prediccion->prediccion_SO2 > LIMITE_SO2_HORA)
    {
        alertas_moderadas++;
    }

    // Evaluar O3
    if (prediccion->prediccion_O3 > LIMITE_O3_8H * 1.5f)
    {
        alertas_criticas++;
    }
    else if (prediccion->prediccion_O3 > LIMITE_O3_8H)
    {
        alertas_moderadas++;
    }

    if (alertas_criticas > 0)
    {
        return 3; // Crítica
    }
    else if (alertas_moderadas > 1)
    {
        return 2; // Alerta
    }
    else
    {
        return 1; // Normal
    }
}

// ========== FUNCIONES DE RECOMENDACIONES ==========

int generar_recomendaciones(DatosZona *zona, IndiceCalidad *indice,
                            PrediccionContaminacion *prediccion,
                            char recomendaciones[10][200],
                            int max_recomendaciones)
{
    int cantidad = 0;

    // Recomendaciones basadas en estado actual
    if (indice->estado_calidad >= 3)
    {
        // Dañina o muy dañina
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[CRITICO] Suspender actividades al aire libre, especialmente para grupos vulnerables");
        }
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[CRÍTICO] Intensificar control de tráfico vehicular en la zona");
        }
        if (cantidad < max_recomendaciones && indice->promedio_PM25 > 55.5f)
        {
            strcpy(recomendaciones[cantidad++],
                   "[CRÍTICO] Considerar cierre temporal de industrias contaminantes");
        }
    }
    else if (indice->estado_calidad == 2)
    {
        // Moderada
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[ALERTA] Limitar actividades al aire libre para niños y adultos mayores");
        }
        if (cantidad < max_recomendaciones && indice->promedio_NO2 > LIMITE_NO2_HORA)
        {
            strcpy(recomendaciones[cantidad++],
                   "[ALERTA] Aumentar control de emisiones vehiculares");
        }
    }

    // Recomendaciones basadas en factores climáticos
    if (zona->cantidad_registros > 0)
    {
        float humedad_promedio = 0, velocidad_promedio = 0;
        int contador = zona->cantidad_registros - 168;
        if (contador < 0)
            contador = 0;

        for (int i = contador; i < zona->cantidad_registros; i++)
        {
            humedad_promedio += zona->registros[i].HUM;
            velocidad_promedio += zona->registros[i].VEL;
        }
        int num_registros = zona->cantidad_registros - contador;
        if (num_registros > 0)
        {
            humedad_promedio /= num_registros;
            velocidad_promedio /= num_registros;
        }

        if (velocidad_promedio < 1.5f && indice->estado_calidad >= 2)
        {
            if (cantidad < max_recomendaciones)
            {
                strcpy(recomendaciones[cantidad++],
                       "[AMBIENTAL] Baja velocidad del viento favorece acumulación de contaminantes");
            }
        }

        if (humedad_promedio > 80 && indice->promedio_PM25 > 25)
        {
            if (cantidad < max_recomendaciones)
            {
                strcpy(recomendaciones[cantidad++],
                       "[AMBIENTAL] Humedad alta puede aumentar formación de contaminantes secundarios");
            }
        }
    }

    // Recomendaciones socioeconómicas
    if (prediccion->alerta_nivel >= 2)
    {
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[SOCIAL] Informar a centros educativos sobre riesgos para la salud");
        }
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[ECONOMICA] Evaluar viabilidad de programas de teletrabajo y educación virtual");
        }
    }

    // Recomendaciones generales de mitigación
    if (indice->estado_calidad >= 2)
    {
        if (cantidad < max_recomendaciones && indice->promedio_CO > 3)
        {
            strcpy(recomendaciones[cantidad++],
                   "[MITIGACION] Ampliar restricción de circulación vehicular (días y horarios)");
        }
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[MITIGACION] Promover uso de transporte público y transporte no motorizado");
        }
        if (cantidad < max_recomendaciones)
        {
            strcpy(recomendaciones[cantidad++],
                   "[MITIGACION] Incrementar riego de vías para reducir material particulado");
        }
    }

    return cantidad;
}

// ========== FUNCIONES DE EXPORTACIÓN ==========

int exportar_reporte_zona(DatosZona *zona, IndiceCalidad *indice,
                          PrediccionContaminacion *prediccion,
                          char recomendaciones[10][200],
                          int cantidad_recomendaciones,
                          const char *nombre_archivo)
{
    FILE *archivo = fopen(nombre_archivo, "a");
    if (archivo == NULL)
    {
        printf("ERROR: No se puede crear el archivo de reporte\n");
        return 0;
    }

    time_t ahora = time(NULL);
    struct tm *info = localtime(&ahora);

    // Encabezado de zona
    fprintf(archivo, "========================================================\n");
    fprintf(archivo, "ZONA: %s\n", zona->nombre);
    fprintf(archivo, "Generado: %02d/%02d/%04d %02d:%02d\n",
            info->tm_mday, info->tm_mon + 1, info->tm_year + 1900,
            info->tm_hour, info->tm_min);
    fprintf(archivo, "========================================================\n\n");

    // Índices actuales
    const char *estado[] = {"BUENA", "MODERADA", "DAÑINA", "MUY DAÑINA"};
    fprintf(archivo, "INDICES DE CONTAMINACION ACTUALES:\n");
    fprintf(archivo, "  NO2 (ppb):           %.2f  (Limite OMS: %.1f) [%s]\n",
            indice->promedio_NO2, LIMITE_NO2_HORA,
            (indice->promedio_NO2 > LIMITE_NO2_HORA) ? "ALERTA" : "OK");
    fprintf(archivo, "  PM2.5 (ug/m3):       %.2f  (Limite OMS: %.1f) [%s]\n",
            indice->promedio_PM25, LIMITE_PM25_24H,
            (indice->promedio_PM25 > LIMITE_PM25_24H) ? "ALERTA" : "OK");
    fprintf(archivo, "  SO2 (ppb):           %.2f  (Limite OMS: %.1f) [%s]\n",
            indice->promedio_SO2, LIMITE_SO2_HORA,
            (indice->promedio_SO2 > LIMITE_SO2_HORA) ? "ALERTA" : "OK");
    fprintf(archivo, "  O3 (ppb):            %.2f  (Limite OMS: %.1f) [%s]\n",
            indice->promedio_O3, LIMITE_O3_8H,
            (indice->promedio_O3 > LIMITE_O3_8H) ? "ALERTA" : "OK");
    fprintf(archivo, "  CO (ppm):            %.2f  (Limite OMS: %.1f) [%s]\n",
            indice->promedio_CO, LIMITE_CO_8H,
            (indice->promedio_CO > LIMITE_CO_8H) ? "ALERTA" : "OK");
    fprintf(archivo, "  Estado de Calidad:   %s\n\n",
            estado[indice->estado_calidad - 1]);

    // Predicciones
    const char *alerta[] = {"NORMAL", "ALERTA", "CRITICA"};
    fprintf(archivo, "PREDICCION - PROXIMAS 24 HORAS:\n");
    fprintf(archivo, "  NO2 (ppb):           %.2f  [%s]\n",
            prediccion->prediccion_NO2,
            (prediccion->prediccion_NO2 > LIMITE_NO2_HORA) ? "ALERTA" : "OK");
    fprintf(archivo, "  PM2.5 (ug/m3):       %.2f  [%s]\n",
            prediccion->prediccion_PM25,
            (prediccion->prediccion_PM25 > LIMITE_PM25_24H) ? "ALERTA" : "OK");
    fprintf(archivo, "  SO2 (ppb):           %.2f  [%s]\n",
            prediccion->prediccion_SO2,
            (prediccion->prediccion_SO2 > LIMITE_SO2_HORA) ? "ALERTA" : "OK");
    fprintf(archivo, "  O3 (ppb):            %.2f  [%s]\n",
            prediccion->prediccion_O3,
            (prediccion->prediccion_O3 > LIMITE_O3_8H) ? "ALERTA" : "OK");
    fprintf(archivo, "  CO (ppm):            %.2f  [%s]\n",
            prediccion->prediccion_CO,
            (prediccion->prediccion_CO > LIMITE_CO_8H) ? "ALERTA" : "OK");
    fprintf(archivo, "  Nivel de Alerta:     %s\n\n",
            alerta[prediccion->alerta_nivel - 1]);

    // Recomendaciones
    if (cantidad_recomendaciones > 0)
    {
        fprintf(archivo, "RECOMENDACIONES:\n");
        for (int i = 0; i < cantidad_recomendaciones; i++)
        {
            // Buscar y remover los brackets [] de la recomendación
            char recom_limpia[200] = "";
            char *inicio_corchete = strchr(recomendaciones[i], '[');
            char *fin_corchete = strchr(recomendaciones[i], ']');

            if (inicio_corchete && fin_corchete && inicio_corchete < fin_corchete)
            {
                // Copiar desde el inicio hasta el corchete
                strncpy(recom_limpia, recomendaciones[i], inicio_corchete - recomendaciones[i]);
                // Agregar el resto después del corchete de cierre
                strcat(recom_limpia, fin_corchete + 1);
                fprintf(archivo, "  %d.%s\n", i + 1, recom_limpia);
            }
            else
            {
                fprintf(archivo, "  %d. %s\n", i + 1, recomendaciones[i]);
            }
        }
        fprintf(archivo, "\n");
    }

    fclose(archivo);
    return 1;
}

int exportar_reporte(DatosZona *zonas, int cantidad_zonas,
                     const char *nombre_archivo)
{
    // Crear archivo nuevo (truncar si existe)
    FILE *archivo = fopen(nombre_archivo, "w");
    if (archivo == NULL)
    {
        printf("ERROR: No se puede crear el archivo de reporte\n");
        return 0;
    }

    fprintf(archivo, "==============================================================\n");
    fprintf(archivo, "   REPORTE GENERAL DE CONTAMINACION ATMOSFERICA\n");
    fprintf(archivo, "                    Quito, Ecuador\n");
    fprintf(archivo, "==============================================================\n\n");

    time_t ahora = time(NULL);
    struct tm *info = localtime(&ahora);
    fprintf(archivo, "Generado: %02d/%02d/%04d %02d:%02d:%02d\n\n",
            info->tm_mday, info->tm_mon + 1, info->tm_year + 1900,
            info->tm_hour, info->tm_min, info->tm_sec);

    // Normativas al inicio
    fprintf(archivo, "NORMATIVA APLICABLE:\n");
    fprintf(archivo, "  - Resolucion 2254-2017: Limites de contaminantes atmosfericos\n");
    fprintf(archivo, "  - Estandares OMS: Estandares de calidad del aire\n");
    fprintf(archivo, "\n");
    fprintf(archivo, "==============================================================\n\n");

    fclose(archivo);

    // Agregar reporte de cada zona
    for (int i = 0; i < cantidad_zonas; i++)
    {
        IndiceCalidad indice = calcular_indice_actual(&zonas[i]);
        PrediccionContaminacion prediccion = predecir_contaminacion_24h(&zonas[i]);

        char recomendaciones[10][200];
        int cantidad_recom = generar_recomendaciones(&zonas[i], &indice, &prediccion,
                                                     recomendaciones, 10);

        exportar_reporte_zona(&zonas[i], &indice, &prediccion,
                              recomendaciones, cantidad_recom, nombre_archivo);
    }

    return 1;
}

// ========== FUNCIONES DE VALIDACIÓN ==========

void limpiar_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int validar_entrada_numerica(char *entrada, int min, int max)
{
    if (entrada == NULL || strlen(entrada) == 0)
    {
        return -1;
    }

    // Verificar que solo contenga dígitos (permitir signo negativo al inicio)
    int es_valido = 1;
    for (int i = 0; entrada[i] != '\0'; i++)
    {
        if (i == 0 && entrada[i] == '-')
        {
            continue;
        }
        if (entrada[i] < '0' || entrada[i] > '9')
        {
            es_valido = 0;
            break;
        }
    }

    if (!es_valido)
    {
        printf("ERROR: Entrada debe ser numerica\n");
        return -1;
    }

    int valor = atoi(entrada);

    if (valor < min || valor > max)
    {
        printf("ERROR: Valor debe estar entre %d y %d\n", min, max);
        return -1;
    }

    return valor;
}

float validar_entrada_float_simple(const char* buffer, float min)
{
    // Eliminar espacios y saltos de línea
    char limpio[50];
    int j = 0;
    for (int i = 0; buffer[i] != '\0' && buffer[i] != '\n'; i++)
    {
        if (buffer[i] != ' ')
        {
            limpio[j++] = buffer[i];
        }
    }
    limpio[j] = '\0';

    // Si está vacío o es solo Enter, retornar 0
    if (strlen(limpio) == 0)
    {
        return 0.0f;
    }

    // Verificar que solo contenga dígitos, punto decimal y opcionalmente signo negativo
    int tiene_punto = 0;
    for (int i = 0; limpio[i] != '\0'; i++)
    {
        if (i == 0 && limpio[i] == '-')
        {
            continue; // Permitir signo negativo al inicio
        }
        if (limpio[i] == '.' && !tiene_punto)
        {
            tiene_punto = 1;
            continue;
        }
        if (limpio[i] < '0' || limpio[i] > '9')
        {
            printf("ERROR: Entrada debe ser numerica. Intente de nuevo.\n");
            return -999999.0f; // Valor especial para indicar error
        }
    }

    float valor = atof(limpio);

    // Validar que no sea negativo si min es 0 o mayor
    if (valor < min)
    {
        printf("ERROR: El valor no puede ser negativo. Intente de nuevo.\n");
        return -999999.0f; // Valor especial para indicar error
    }

    return valor;
}

// ========== FUNCIONES DE INTERFAZ ==========

int mostrar_menu_principal(void)
{
    printf("\n");
    printf("==============================================================\n");
    printf("  SISTEMA DE MONITOREO DE CONTAMINACION ATMOSFERICA\n");
    printf("              QUITO - ECUADOR\n");
    printf("==============================================================\n");
    printf("\n");
    printf("  1. Analizar una zona\n");
    printf("  2. Ver estado de alertas en todas las zonas\n");
    printf("  3. Generar reporte completo\n");
    printf("  4. Ingresar datos del dia\n");
    printf("  5. Salir\n");
    printf("\nSeleccione opcion: ");

    char entrada[50];
    if (fgets(entrada, sizeof(entrada), stdin) == NULL)
    {
        return -1;
    }

    // Remover salto de línea
    entrada[strcspn(entrada, "\n")] = 0;

    return validar_entrada_numerica(entrada, 1, 5);
}

int mostrar_menu_zonas(void)
{
    printf("\n");
    printf("=============================================\n");
    printf("         SELECCIONE UNA ZONA\n");
    printf("=============================================\n");
    printf("\n");
    printf("  1. Belisario\n");
    printf("  2. Carapungo\n");
    printf("  3. Centro\n");
    printf("  4. Cotocollao\n");
    printf("  5. Elcamal\n");
    printf("  6. Volver al menu principal\n");
    printf("\nSeleccione opcion: ");

    char entrada[50];
    if (fgets(entrada, sizeof(entrada), stdin) == NULL)
    {
        return -1;
    }

    entrada[strcspn(entrada, "\n")] = 0;

    int opcion = validar_entrada_numerica(entrada, 1, 6);
    if (opcion == -1)
    {
        return -1;
    }

    if (opcion == 6)
    {
        return -1;
    }

    return opcion - 1;
}

int mostrar_menu_zona_analisis(void)
{
    printf("\n");
    printf("=============================================\n");
    printf("          ANALISIS DE ZONA\n");
    printf("=============================================\n");
    printf("\n");
    printf("  1. Ver indice de contaminacion actual\n");
    printf("  2. Ver prediccion para las proximas 24h\n");
    printf("  3. Ver promedio historico (ultimos 30 dias)\n");
    printf("  4. Ver recomendaciones\n");
    printf("  5. Volver\n");
    printf("\nSeleccione opcion: ");

    char entrada[50];
    if (fgets(entrada, sizeof(entrada), stdin) == NULL)
    {
        return -1;
    }

    entrada[strcspn(entrada, "\n")] = 0;

    int opcion = validar_entrada_numerica(entrada, 1, 5);
    if (opcion == -1)
    {
        return -1;
    }

    return opcion;
}

void imprimir_indice_calidad(IndiceCalidad *indice, int tipo)
{
    const char *titulo = (tipo == 0) ? "INDICES ACTUALES (Ultimos 7 dias)" : "PROMEDIO HISTORICO (Ultimos 30 dias)";
    const char *estado[] = {"BUENA", "MODERADA", "DAÑINA", "MUY DAÑINA"};

    printf("\n");
    printf("========================================================\n");
    printf("%s\n", titulo);
    printf("========================================================\n");
    printf("\n");

    printf("  NO2 (ppb):              %.2f\n", indice->promedio_NO2);
    printf("    Limite OMS:         %.1f (Alerta si > %.1f)\n\n",
           LIMITE_NO2_HORA, LIMITE_NO2_HORA);

    printf("  PM2.5 (ug/m3):          %.2f\n", indice->promedio_PM25);
    printf("    Limite OMS:         %.1f (Alerta si > %.1f)\n\n",
           LIMITE_PM25_24H, LIMITE_PM25_24H);

    printf("  SO2 (ppb):              %.2f\n", indice->promedio_SO2);
    printf("    Limite OMS:         %.1f (Alerta si > %.1f)\n\n",
           LIMITE_SO2_HORA, LIMITE_SO2_HORA);

    printf("  O3 (ppb):               %.2f\n", indice->promedio_O3);
    printf("    Limite OMS:         %.1f (Alerta si > %.1f)\n\n",
           LIMITE_O3_8H, LIMITE_O3_8H);

    printf("  CO (ppm):               %.2f\n", indice->promedio_CO);
    printf("    Limite OMS:         %.1f\n\n", LIMITE_CO_8H);

    printf("  Estado de Calidad:      %s\n", estado[indice->estado_calidad - 1]);
    printf("\n");
}

void imprimir_prediccion(PrediccionContaminacion *prediccion)
{
    printf("\n");
    printf("========================================================\n");
    printf("      PREDICCION - PROXIMAS 24 HORAS\n");
    printf("========================================================\n");
    printf("\n");

    printf("  NO2 (ppb):              %.2f\n", prediccion->prediccion_NO2);
    printf("  PM2.5 (ug/m3):          %.2f\n", prediccion->prediccion_PM25);
    printf("  SO2 (ppb):              %.2f\n", prediccion->prediccion_SO2);
    printf("  O3 (ppb):               %.2f\n", prediccion->prediccion_O3);
    printf("  CO (ppm):               %.2f\n\n", prediccion->prediccion_CO);

    const char *alerta[] = {"NORMAL", "ALERTA", "CRITICA"};
    const char *color_inicio = (prediccion->alerta_nivel == 3) ? "[CRITICA] " : (prediccion->alerta_nivel == 2) ? "[ALERTA] "
                                                                                                                : "[OK] ";

    printf("  Nivel de Alerta:        %s%s\n\n", color_inicio, alerta[prediccion->alerta_nivel - 1]);
}

void imprimir_recomendaciones(char recomendaciones[10][200], int cantidad)
{
    printf("\n");
    printf("========================================================\n");
    printf("           RECOMENDACIONES\n");
    printf("========================================================\n");
    printf("\n");

    if (cantidad == 0)
    {
        printf("  No hay recomendaciones en este momento.\n");
        printf("  Calidad del aire es BUENA.\n\n");
        return;
    }

    for (int i = 0; i < cantidad; i++)
    {
        printf("  %d. %s\n", i + 1, recomendaciones[i]);
    }
    printf("\n");
}

void imprimir_estado_alertas(DatosZona *zonas, int cantidad_zonas)
{
    printf("\n");
    printf("========================================================\n");
    printf("     ESTADO DE ALERTAS - TODAS LAS ZONAS\n");
    printf("========================================================\n");
    printf("\n");

    printf("%-18s %-18s %-18s %-18s\n", "ZONA", "ALERTA ACTUAL", "PREDICCION 24h", "ESTADO");
    printf("%-18s %-18s %-18s %-18s\n", "------------------", "------------------", "------------------", "------------------");

    for (int i = 0; i < cantidad_zonas; i++)
    {
        IndiceCalidad indice = calcular_indice_actual(&zonas[i]);
        PrediccionContaminacion prediccion = predecir_contaminacion_24h(&zonas[i]);

        const char *alerta_actual[] = {"NORMAL", "ALERTA", "CRITICA"};
        const char *estado_calidad[] = {"BUENA", "MODERADA", "DAÑINA", "MUY DAÑINA"};

        // Evaluar alerta actual basada en índice
        int alerta_actual_nivel = 1;
        if (indice.promedio_PM25 > LIMITE_PM25_24H * 1.5f ||
            indice.promedio_NO2 > LIMITE_NO2_HORA * 1.5f)
        {
            alerta_actual_nivel = 3;
        }
        else if (indice.promedio_PM25 > LIMITE_PM25_24H ||
                 indice.promedio_NO2 > LIMITE_NO2_HORA)
        {
            alerta_actual_nivel = 2;
        }

        printf("%-18s %-18s %-18s %-18s\n",
               zonas[i].nombre,
               alerta_actual[alerta_actual_nivel - 1],
               alerta_actual[prediccion.alerta_nivel - 1],
               estado_calidad[indice.estado_calidad - 1]);
    }
    printf("\n");
}

// ========== FUNCIONES DE LECTURA DE REPORTES ==========

int leer_reporte_contaminacion(const char *ruta)
{
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL)
    {
        printf("ERROR: No se puede abrir el archivo de reporte: %s\n", ruta);
        return 0;
    }

    char linea[512];
    char zona_actual[50] = "";
    int numero_recomendacion = 0;

    printf("\n");
    printf("========================================================\n");
    printf("          REPORTE DE CONTAMINACION\n");
    printf("========================================================\n");
    printf("\n");

    while (fgets(linea, sizeof(linea), archivo) != NULL)
    {
        // Remover salto de línea
        linea[strcspn(linea, "\n")] = 0;

        // Saltar líneas vacías y comentarios de inicio
        if (strlen(linea) == 0 || (linea[0] == '#' && strstr(linea, "ZONA") == NULL &&
                                   strstr(linea, "Generado") == NULL))
        {
            continue;
        }

        // Detectar nueva zona
        if (strstr(linea, "# ZONA") != NULL)
        {
            sscanf(linea, "# ZONA %s", zona_actual);
            numero_recomendacion = 0;
            printf("\n--------------------------------------------------\n");
            printf("ZONA: %s\n", zona_actual);
            printf("--------------------------------------------------\n");
            continue;
        }

        // Saltar encabezados generados
        if (strstr(linea, "# Generado") != NULL || strstr(linea, "# Formato") != NULL)
        {
            continue;
        }

        // Procesar líneas de índice actual
        if (strstr(linea, "INDICE_ACTUAL") != NULL)
        {
            char parametro[50], estado[50];
            float valor, limite;

            if (sscanf(linea, "INDICE_ACTUAL %s %f %f %s", parametro, &valor, &limite, estado) == 4)
            {
                printf("  %-15s: %.2f (Limite: %.1f) [%s]\n", parametro, valor, limite, estado);
            }
            continue;
        }

        // Procesar líneas de predicción
        if (strstr(linea, "PREDICCION_24H") != NULL)
        {
            if (strcmp(linea, "PREDICCION_24H") == 0 || strstr(linea, "PREDICCION_24H") == NULL)
            {
                printf("\n  Prediccion 24 horas:\n");
            }
            else
            {
                char parametro[50], estado[50];
                float valor, limite;

                if (sscanf(linea, "PREDICCION_24H %s %f %f %s", parametro, &valor, &limite, estado) == 4)
                {
                    printf("    %-15s: %.2f (Limite: %.1f) [%s]\n", parametro, valor, limite, estado);
                }
            }
            continue;
        }

        // Procesar recomendaciones
        if (strstr(linea, "RECOMENDACION") != NULL)
        {
            char recomendacion[200];
            int num;

            if (sscanf(linea, "RECOMENDACION %d %[^\n]", &num, recomendacion) == 2)
            {
                printf("  %d. %s\n", num, recomendacion);
                numero_recomendacion++;
            }
            continue;
        }

        // Procesar normativa
        if (strstr(linea, "NORMATIVA") != NULL)
        {
            char norma[100], descripcion[100];

            if (sscanf(linea, "NORMATIVA %s %s", norma, descripcion) == 2)
            {
                if (numero_recomendacion > 0)
                {
                    printf("\n  Normativa aplicable: %s - %s\n", norma, descripcion);
                }
            }
            continue;
        }
    }

    fclose(archivo);
    printf("\n========================================================\n\n");
    return 1;
}

// ========== FUNCIONES DE INGRESO DE DATOS ==========

int ingresar_datos_dia(DatosZona *zonas, int cantidad_zonas)
{
    printf("\n");
    printf("==============================================================\n");
    printf("           INGRESO DE DATOS DEL DIA\n");
    printf("==============================================================\n");
    printf("\n");

    // Seleccionar sector/zona
    printf("Seleccione el sector:\n");
    for (int i = 0; i < cantidad_zonas; i++)
    {
        printf("  %d. %s\n", i + 1, zonas[i].nombre);
    }
    printf("  0. Cancelar\n");
    printf("\nOpcion: ");

    char buffer[50];
    fgets(buffer, sizeof(buffer), stdin);
    int zona_seleccionada = atoi(buffer);

    if (zona_seleccionada == 0)
    {
        printf("Operacion cancelada.\n");
        return 0;
    }

    if (zona_seleccionada < 1 || zona_seleccionada > cantidad_zonas)
    {
        printf("ERROR: Sector invalido.\n");
        return 0;
    }

    int zona_idx = zona_seleccionada - 1;
    DatosZona *zona = &zonas[zona_idx];

    // Verificar que no se exceda el límite de registros
    if (zona->cantidad_registros >= MAX_REGISTROS)
    {
        printf("ERROR: No hay espacio para mas registros en esta zona.\n");
        return 0;
    }

    printf("\n--------------------------------------------------------------\n");
    printf("Ingresando datos para: %s\n", zona->nombre);
    printf("--------------------------------------------------------------\n");
    printf("Instrucciones:\n");
    printf("  - Ingrese el valor de cada indice\n");
    printf("  - Si el indice no aplica o no tiene dato, ingrese 0\n");
    printf("  - Puede presionar Enter para usar 0 como valor predeterminado\n");
    printf("\n");

    RegistroContaminacion nuevo_registro;

    // Obtener fecha y hora actual
    time_t ahora = time(NULL);
    struct tm *info = localtime(&ahora);
    snprintf(nuevo_registro.fecha, sizeof(nuevo_registro.fecha),
             "%04d-%02d-%02d",
             info->tm_year + 1900,
             info->tm_mon + 1,
             info->tm_mday);
    snprintf(nuevo_registro.hora, sizeof(nuevo_registro.hora),
             "%02d:00:00",
             info->tm_hour);

    printf("Fecha y hora del registro: %s %s\n\n", nuevo_registro.fecha, nuevo_registro.hora);

    // Ingresar CO (Monóxido de Carbono)
    float valor_temp;
    do {
        printf("CO (Monoxido de Carbono) [mg/m3]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.CO = valor_temp;

    // Ingresar NO2 (Dióxido de Nitrógeno)
    do {
        printf("NO2 (Dioxido de Nitrogeno) [ug/m3]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.NO2 = valor_temp;

    // Ingresar O3 (Ozono)
    do {
        printf("O3 (Ozono) [ug/m3]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.O3 = valor_temp;

    // Ingresar PM2.5 (Material Particulado)
    do {
        printf("PM2.5 (Material Particulado) [ug/m3]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.PM25 = valor_temp;

    // Ingresar SO2 (Dióxido de Azufre)
    do {
        printf("SO2 (Dioxido de Azufre) [ug/m3]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.SO2 = valor_temp;

    printf("\n--- Datos Meteorologicos ---\n");

    // Ingresar Temperatura
    do {
        printf("TMP (Temperatura) [C]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, -50.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.TMP = valor_temp;

    // Ingresar Humedad
    do {
        printf("HUM (Humedad) [%%]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.HUM = valor_temp;

    // Ingresar Lluvia
    do {
        printf("LLU (Lluvia) [mm]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.LLU = valor_temp;

    // Ingresar Velocidad del viento
    do {
        printf("VEL (Velocidad del viento) [m/s]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.VEL = valor_temp;

    // Ingresar Dirección del viento
    do {
        printf("DIR (Direccion del viento) [grados]: ");
        fgets(buffer, sizeof(buffer), stdin);
        valor_temp = validar_entrada_float_simple(buffer, 0.0f);
    } while (valor_temp == -999999.0f);
    nuevo_registro.DIR = valor_temp;

    // Agregar el nuevo registro a la zona
    zona->registros[zona->cantidad_registros] = nuevo_registro;
    zona->cantidad_registros++;

    printf("\n");
    printf("--------------------------------------------------------------\n");
    printf("                  DATOS INGRESADOS EXITOSAMENTE\n");
    printf("--------------------------------------------------------------\n");
    printf("\n");

    // Mostrar resumen de datos ingresados
    printf("Resumen del registro:\n");
    printf("  Fecha/Hora: %s %s\n", nuevo_registro.fecha, nuevo_registro.hora);
    printf("  CO:   %.2f mg/m3\n", nuevo_registro.CO);
    printf("  NO2:  %.2f ug/m3\n", nuevo_registro.NO2);
    printf("  O3:   %.2f ug/m3\n", nuevo_registro.O3);
    printf("  PM2.5: %.2f ug/m3\n", nuevo_registro.PM25);
    printf("  SO2:  %.2f ug/m3\n", nuevo_registro.SO2);
    printf("  Temperatura: %.1f C\n", nuevo_registro.TMP);
    printf("  Humedad: %.1f%%\n", nuevo_registro.HUM);
    printf("  Lluvia: %.1f mm\n", nuevo_registro.LLU);
    printf("  Viento: %.1f m/s, %.0f grados\n", nuevo_registro.VEL, nuevo_registro.DIR);

    // Pedir confirmación antes de guardar
    printf("\n");
    printf("--------------------------------------------------------------\n");
    printf("Desea guardar estos datos en el archivo? (s/n): ");
    
    char confirmacion[10];
    fgets(confirmacion, sizeof(confirmacion), stdin);
    confirmacion[strcspn(confirmacion, "\n")] = 0;
    
    // Convertir a minúsculas para comparación
    if (confirmacion[0] != 's' && confirmacion[0] != 'S' && 
        strcmp(confirmacion, "si") != 0 && strcmp(confirmacion, "SI") != 0 &&
        strcmp(confirmacion, "Si") != 0)
    {
        printf("\nOperacion cancelada, los datos no se han guardado.\n");
        
        // Revertir cambios en memoria (no se había agregado al archivo aún)
        zona->cantidad_registros--;
        
        return 0;
    }

    // Calcular y mostrar índice de calidad actual con el nuevo dato
    printf("\n");
    printf("--------------------------------------------------------------\n");
    printf("              CALCULO DE INDICES ACTUALIZADOS\n");
    printf("--------------------------------------------------------------\n");
    printf("\n");

    IndiceCalidad indice_actualizado = calcular_indice_actual(zona);
    printf("Indice de Calidad del Aire (ultimos 7 dias):\n");
    printf("  Promedio NO2:  %.2f ug/m3\n", indice_actualizado.promedio_NO2);
    printf("  Promedio PM2.5: %.2f ug/m3\n", indice_actualizado.promedio_PM25);
    printf("  Promedio SO2:  %.2f ug/m3\n", indice_actualizado.promedio_SO2);
    printf("  Promedio O3:   %.2f ug/m3\n", indice_actualizado.promedio_O3);
    printf("  Promedio CO:   %.2f mg/m3\n", indice_actualizado.promedio_CO);

    const char *estados[] = {"", "BUENA", "MODERADA", "DAÑINA", "MUY DAÑINA"};
    printf("  Estado de Calidad: %s\n", estados[indice_actualizado.estado_calidad]);

    // Calcular y mostrar predicción para las próximas 24 horas
    printf("\n");
    printf("Prediccion para las proximas 24 horas:\n");
    PrediccionContaminacion prediccion = predecir_contaminacion_24h(zona);
    printf("  NO2 predicho:  %.2f ug/m3\n", prediccion.prediccion_NO2);
    printf("  PM2.5 predicho: %.2f ug/m3\n", prediccion.prediccion_PM25);
    printf("  SO2 predicho:  %.2f ug/m3\n", prediccion.prediccion_SO2);
    printf("  O3 predicho:   %.2f ug/m3\n", prediccion.prediccion_O3);
    printf("  CO predicho:   %.2f mg/m3\n", prediccion.prediccion_CO);

    const char *niveles_alerta[] = {"", "NORMAL", "ALERTA", "CRITICA"};
    printf("  Nivel de Alerta: %s\n", niveles_alerta[prediccion.alerta_nivel]);

    // Calcular promedio histórico (30 días)
    printf("\n");
    printf("Promedio Historico (ultimos 30 dias):\n");
    IndiceCalidad historico = calcular_promedio_historico(zona);
    printf("  Promedio NO2:  %.2f ug/m3\n", historico.promedio_NO2);
    printf("  Promedio PM2.5: %.2f ug/m3\n", historico.promedio_PM25);
    printf("  Promedio SO2:  %.2f ug/m3\n", historico.promedio_SO2);
    printf("  Promedio O3:   %.2f ug/m3\n", historico.promedio_O3);
    printf("  Promedio CO:   %.2f mg/m3\n", historico.promedio_CO);
    printf("  Estado de Calidad: %s\n", estados[historico.estado_calidad]);

    printf("\n");
    printf("--------------------------------------------------------------\n");
    printf("          CONFIRMACION DE GUARDADO DE DATOS\n");
    printf("--------------------------------------------------------------\n");
    printf("\n");

    // Guardar en archivo
    char nombre_archivo[150];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "%s_contaminacion.data", zona->nombre);

    FILE *archivo = fopen(nombre_archivo, "a");
    if (archivo != NULL)
    {
        fprintf(archivo, "%s %s %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
                nuevo_registro.fecha,
                nuevo_registro.hora,
                nuevo_registro.CO,
                nuevo_registro.DIR,
                nuevo_registro.HUM,
                nuevo_registro.LLU,
                nuevo_registro.NO2,
                nuevo_registro.O3,
                nuevo_registro.PM25,
                nuevo_registro.SO2,
                nuevo_registro.TMP,
                nuevo_registro.VEL);
        fclose(archivo);
        printf("[OK] Datos guardados en archivo: %s\n", nombre_archivo);
    }
    else
    {
        printf("[ADVERTENCIA] No se pudo guardar en archivo, pero los datos\n");
        printf("              permanecen en memoria durante esta sesion.\n");
    }

    return 1;
}
