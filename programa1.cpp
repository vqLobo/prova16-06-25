#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <strings.h>

#define MAX_SENSORS 50
#define MAX_ID_LEN 17
#define MAX_VALUE_LEN 17
#define INITIAL_CAPACITY 100
#define MAX_LINE_LEN 256

typedef enum {
    T_INT,
    T_FLOAT,
    T_BOOL,
    T_STRING,
    T_UNKNOWN
} DataType;

typedef struct {
    long timestamp;
    union {
        int int_val;
        double float_val;
        bool bool_val;
        char str_val[MAX_VALUE_LEN];
    } value;
} SensorReading;

typedef struct {
    char sensor_id[MAX_ID_LEN];
    DataType type;
    SensorReading *readings;
    int count;
    int capacity;
} SensorData;

DataType determine_data_type(const char *value_str) {
    if (strcasecmp(value_str, "true") == 0 || strcasecmp(value_str, "false") == 0) {
        return T_BOOL;
    }
    
    char *endptr_int;
    strtol(value_str, &endptr_int, 10);
    if (*endptr_int == '\0') {
        return T_INT;
    }
    
    char *endptr_float;
    strtod(value_str, &endptr_float);
    if (*endptr_float == '\0' && endptr_float != value_str) {
        return T_FLOAT;
    }
    
    size_t len = strlen(value_str);
    if (len > 0 && len <= MAX_VALUE_LEN - 1) {
        return T_STRING;
    }

    return T_UNKNOWN;
}

int compare_readings_desc(const void *a, const void *b) {
    const SensorReading *r1 = (const SensorReading *)a;
    const SensorReading *r2 = (const SensorReading *)b;
    return (r2->timestamp > r1->timestamp) - (r2->timestamp < r1->timestamp);
}

void process_value(SensorReading *reading, DataType type, const char *value_str) {
    switch (type) {
        case T_INT:
            reading->value.int_val = atoi(value_str);
            break;
        case T_FLOAT:
            reading->value.float_val = atof(value_str);
            break;
        case T_BOOL:
            reading->value.bool_val = (strcasecmp(value_str, "true") == 0);
            break;
        case T_STRING:
            strncpy(reading->value.str_val, value_str, MAX_VALUE_LEN - 1);
            reading->value.str_val[MAX_VALUE_LEN - 1] = '\0';
            break;
        case T_UNKNOWN:
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_de_entrada>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s dados_sensores.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return EXIT_FAILURE;
    }

    SensorData sensors[MAX_SENSORS] = {0};
    int sensor_count = 0;
    char line[MAX_LINE_LEN];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (line[0] == '\0') {
            continue;
        }

        long timestamp;
        char sensor_id[MAX_ID_LEN];
        char value_str[MAX_VALUE_LEN];

        if (sscanf(line, "%ld %16s %16s", &timestamp, sensor_id, value_str) != 3) {
            fprintf(stderr, "Formato inválido na linha %d: %s\n", line_number, line);
            continue;
        }

        if (timestamp < 946684800 || timestamp > 4102444800) {
            fprintf(stderr, "Timestamp inválido na linha %d: %ld\n", line_number, timestamp);
            continue;
        }

        int sensor_idx = -1;
        for (int i = 0; i < sensor_count; i++) {
            if (strcmp(sensors[i].sensor_id, sensor_id) == 0) {
                sensor_idx = i;
                break;
            }
        }

        if (sensor_idx == -1) {
            if (sensor_count >= MAX_SENSORS) {
                fprintf(stderr, "Limite máximo de sensores atingido. Ignorando sensor '%s'\n", sensor_id);
                continue;
            }
            
            sensor_idx = sensor_count++;
            strncpy(sensors[sensor_idx].sensor_id, sensor_id, MAX_ID_LEN - 1);
            sensors[sensor_idx].sensor_id[MAX_ID_LEN - 1] = '\0';
            
            sensors[sensor_idx].type = determine_data_type(value_str);
            if (sensors[sensor_idx].type == T_UNKNOWN) {
                fprintf(stderr, "Tipo de valor inválido para o sensor '%s': %s\n", sensor_id, value_str);
                sensor_count--;
                continue;
            }

            sensors[sensor_idx].count = 0;
            sensors[sensor_idx].capacity = INITIAL_CAPACITY;
            sensors[sensor_idx].readings = (SensorReading *)malloc(INITIAL_CAPACITY * sizeof(SensorReading));
            if (!sensors[sensor_idx].readings) {
                perror("Falha na alocação de memória");
                exit(EXIT_FAILURE);
            }
        }

        DataType current_type = determine_data_type(value_str);
        if (current_type != sensors[sensor_idx].type) {
            fprintf(stderr, "Tipo inconsistente para o sensor '%s'. Esperado: %d, Obtido: %d\n",
                    sensor_id, sensors[sensor_idx].type, current_type);
            continue;
        }

        if (sensors[sensor_idx].count >= sensors[sensor_idx].capacity) {
            sensors[sensor_idx].capacity *= 2;
            SensorReading *new_readings = (SensorReading *)realloc(sensors[sensor_idx].readings,
                                                sensors[sensor_idx].capacity * sizeof(SensorReading));
            if (!new_readings) {
                perror("Falha na realocação de memória");
                exit(EXIT_FAILURE);
            }
            sensors[sensor_idx].readings = new_readings;
        }

        SensorReading *reading = &sensors[sensor_idx].readings[sensors[sensor_idx].count];
        reading->timestamp = timestamp;
        process_value(reading, sensors[sensor_idx].type, value_str);
        sensors[sensor_idx].count++;
    }
    fclose(file);

    printf("\nProcessamento concluído. Criando arquivos de saída em ordem decrescente...\n");

    for (int i = 0; i < sensor_count; i++) {
        qsort(sensors[i].readings, sensors[i].count, sizeof(SensorReading), compare_readings_desc);

        char out_filename[MAX_ID_LEN + 5];
        snprintf(out_filename, sizeof(out_filename), "%s.txt", sensors[i].sensor_id);

        FILE *out_file = fopen(out_filename, "w");
        if (!out_file) {
            perror("Erro ao criar arquivo de saída");
            continue;
        }

        for (int j = 0; j < sensors[i].count; j++) {
            fprintf(out_file, "%ld ", sensors[i].readings[j].timestamp);

            switch (sensors[i].type) {
                case T_INT:
                    fprintf(out_file, "%d\n", sensors[i].readings[j].value.int_val);
                    break;
                case T_FLOAT:
                    fprintf(out_file, "%.2f\n", sensors[i].readings[j].value.float_val);
                    break;
                case T_BOOL:
                    fprintf(out_file, "%s\n", sensors[i].readings[j].value.bool_val ? "true" : "false");
                    break;
                case T_STRING:
                    fprintf(out_file, "%s\n", sensors[i].readings[j].value.str_val);
                    break;
                case T_UNKNOWN:
                    break;
            }
        }
        fclose(out_file);

        const char *type_str = "desconhecido";
        switch (sensors[i].type) {
            case T_INT: type_str = "inteiros"; break;
            case T_FLOAT: type_str = "decimais"; break;
            case T_BOOL: type_str = "booleanos"; break;
            case T_STRING: type_str = "strings"; break;
            case T_UNKNOWN: break;
        }

        printf("  - Arquivo '%s' criado com %d leituras (%s) em ordem decrescente\n",
               out_filename, sensors[i].count, type_str);
        
        free(sensors[i].readings);
    }

    printf("\nProcessamento finalizado com sucesso!\n");
    return EXIT_SUCCESS;
}