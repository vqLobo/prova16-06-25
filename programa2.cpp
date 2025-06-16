#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#define MAX_ID_LEN 17
#define MAX_VALUE_LEN 17
#define MAX_LINE_LEN 256

typedef enum {
    T_INT, T_FLOAT, T_BOOL, T_STRING, T_UNKNOWN
} DataType;

typedef struct {
    long timestamp;
    DataType type;
    union {
        int int_val;
        double float_val;
        bool bool_val;
        char str_val[MAX_VALUE_LEN];
    } value;
} SensorReading;

DataType determine_data_type(const char *value_str) {
    if (strcasecmp(value_str, "true") == 0 || strcasecmp(value_str, "false") == 0) {
        return T_BOOL;
    }
    
    char *endptr;
    strtol(value_str, &endptr, 10);
    if (*endptr == '\0') {
        return T_INT;
    }
    
    strtod(value_str, &endptr);
    if (*endptr == '\0') {
        return T_FLOAT;
    }
    
    return T_STRING;
}

int binary_search_closest_desc(SensorReading *readings, int count, long target) {
    if (count == 0) return -1;

    int left = 0;
    int right = count - 1;
    int mid;
    int closest_idx = 0;
    long min_diff = labs(readings[0].timestamp - target);

    while (left <= right) {
        mid = left + (right - left) / 2;
        long diff = labs(readings[mid].timestamp - target);

        if (diff < min_diff) {
            min_diff = diff;
            closest_idx = mid;
        }

        if (readings[mid].timestamp > target) {
            left = mid + 1;
        } else if (readings[mid].timestamp < target) {
            right = mid - 1;
        } else {
            return mid;
        }
    }

    if (closest_idx > 0) {
        long prev_diff = labs(readings[closest_idx-1].timestamp - target);
        if (prev_diff < min_diff) {
            return closest_idx - 1;
        }
    }
    if (closest_idx < count - 1) {
        long next_diff = labs(readings[closest_idx+1].timestamp - target);
        if (next_diff < min_diff) {
            return closest_idx + 1;
        }
    }

    return closest_idx;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <sensor_id> <timestamp>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s TEMP 1630000000\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *sensor_id = argv[1];
    long target_ts = atol(argv[2]);

    if (target_ts < 946684800 || target_ts > 4102444800) {
        fprintf(stderr, "Timestamp invalido! Deve estar entre 2000-01-01 e 2100-01-01.\n");
        return EXIT_FAILURE;
    }

    char filename[MAX_ID_LEN + 5];
    snprintf(filename, sizeof(filename), "%s.txt", sensor_id);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo do sensor");
        return EXIT_FAILURE;
    }

    int count = 0;
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    rewind(file);

    if (count == 0) {
        printf("Arquivo do sensor '%s' esta vazio.\n", sensor_id);
        fclose(file);
        return EXIT_SUCCESS;
    }

    SensorReading *readings = (SensorReading *)malloc(count * sizeof(SensorReading));
    if (!readings) {
        perror("Erro ao alocar memoria");
        fclose(file);
        return EXIT_FAILURE;
    }

    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        char value_str[MAX_VALUE_LEN];
        if (sscanf(line, "%ld %16s", &readings[index].timestamp, value_str) != 2) {
            count--;
            continue;
        }

        readings[index].type = determine_data_type(value_str);
        switch (readings[index].type) {
            case T_INT:    readings[index].value.int_val = atoi(value_str); break;
            case T_FLOAT:  readings[index].value.float_val = atof(value_str); break;
            case T_BOOL:   readings[index].value.bool_val = (strcasecmp(value_str, "true") == 0); break;
            case T_STRING: strncpy(readings[index].value.str_val, value_str, MAX_VALUE_LEN); break;
            case T_UNKNOWN: break;
        }
        index++;
    }
    fclose(file);

    count = index;

    int closest_index = binary_search_closest_desc(readings, count, target_ts);

    printf("\nLeitura mais proxima encontrada para o sensor '%s':\n", sensor_id);
    printf("--------------------------------------------\n");
    printf("Timestamp: %ld\n", readings[closest_index].timestamp);
    printf("Valor: ");
    
    switch (readings[closest_index].type) {
        case T_INT:
            printf("%d (Inteiro)\n", readings[closest_index].value.int_val);
            break;
        case T_FLOAT:
            printf("%.2f (Decimal)\n", readings[closest_index].value.float_val);
            break;
        case T_BOOL:
            printf("%s (Booleano)\n", readings[closest_index].value.bool_val ? "true" : "false");
            break;
        case T_STRING:
            printf("%s (String)\n", readings[closest_index].value.str_val);
            break;
        case T_UNKNOWN:
            printf("Tipo desconhecido\n");
            break;
    }
    
    long diff = labs(readings[closest_index].timestamp - target_ts);
    printf("Diferenca temporal: %ld segundos\n", diff);
    printf("--------------------------------------------\n");

    free(readings);
    return EXIT_SUCCESS;
}