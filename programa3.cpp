//programa 3 pra prova

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define LEITURAS_POR_SENSOR 2000
#define MAX_ID_LEN 17
#define MAX_VALOR_LEN 17
#define MAX_STR_ALEATORIA 16

typedef struct {
    time_t timestamp;
    char sensor_id[MAX_ID_LEN];
    char valor[MAX_VALOR_LEN];
} Leitura;

typedef enum {
    T_INT, T_FLOAT, T_BOOL, T_STRING, T_INVALIDO
} TipoDado;

time_t parse_datetime(const char *data_str, const char *hora_str) {
    struct tm t = {0};
    int dia, mes, ano;
    int hora, minuto, segundo;

    if (sscanf(data_str, "%d-%d-%d", &dia, &mes, &ano) != 3) {
        return -1;
    }

    if (sscanf(hora_str, "%d:%d:%d", &hora, &minuto, &segundo) != 3) {
        return -1;
    }

    t.tm_mday = dia;
    t.tm_mon = mes - 1;
    t.tm_year = ano - 1900;
    t.tm_hour = hora;
    t.tm_min = minuto;
    t.tm_sec = segundo;

    return mktime(&t);
}

TipoDado string_para_tipo(const char *tipo_str) {
    if (strcasecmp(tipo_str, "CONJ_Z") == 0) return T_INT;
    if (strcasecmp(tipo_str, "CONJ_Q") == 0) return T_FLOAT;
    if (strcasecmp(tipo_str, "BINARIO") == 0) return T_BOOL;
    if (strcasecmp(tipo_str, "TEXTO") == 0) return T_STRING;
    return T_INVALIDO;
}

void gerar_valor_aleatorio(char *buffer, TipoDado tipo) {
    switch (tipo) {
        case T_INT:
            snprintf(buffer, MAX_VALOR_LEN, "%d", rand() % 10000);
            break;
        case T_FLOAT:
            snprintf(buffer, MAX_VALOR_LEN, "%.2f", ((double)rand() / RAND_MAX) * 500.0);
            break;
        case T_BOOL:
            snprintf(buffer, MAX_VALOR_LEN, "%s", (rand() % 2 == 0) ? "false" : "true");
            break;
        case T_STRING: {
            const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            int len = 1 + rand() % (MAX_STR_ALEATORIA - 1);
            for (int i = 0; i < len; i++) {
                buffer[i] = charset[rand() % (sizeof(charset) - 1)];
            }
            buffer[len] = '\0';
            break;
        }
        case T_INVALIDO:
            strncpy(buffer, "ERRO", MAX_VALOR_LEN);
            break;
    }
}

void embaralhar(Leitura *v, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Leitura tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 7 || (argc - 5) % 2 != 0) {
        fprintf(stderr, "Uso correto:\n");
        fprintf(stderr, "%s <data_inicio> <hora_inicio> <data_fim> <hora_fim> <sensor1> <tipo1> [<sensor2> <tipo2> ...]\n", argv[0]);
        fprintf(stderr, "data: DD-MM-AAAA\n");
        fprintf(stderr, "hora: HH:MM:SS\n");
        fprintf(stderr, "Tipos validos:\n");
        fprintf(stderr, "  CONJ_Z   - para dados tipo inteiro\n");
        fprintf(stderr, "  CONJ_Q   - para dados do tipo float\n");
        fprintf(stderr, "  TEXTO    - para dados do tipo string\n");
        fprintf(stderr, "  BINARIO  - para dados do tipo booleano\n");
        fprintf(stderr, "Exemplo: %s 15-06-2025 08:00:00 15-06-2025 18:00:00 TEMP CONJ_Q UMID CONJ_Z\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    time_t ts_inicio = parse_datetime(argv[1], argv[2]);
    time_t ts_fim = parse_datetime(argv[3], argv[4]);

    if (ts_inicio == (time_t)-1 || ts_fim == (time_t)-1) {
        fprintf(stderr, "Erro: Formato de data/hora inválido!\n");
        fprintf(stderr, "Use DD-MM-AAAA para datas e HH:MM:SS para horas\n");
        return EXIT_FAILURE;
    }

    if (ts_fim <= ts_inicio) {
        fprintf(stderr, "Erro: A data/hora final deve ser posterior à inicial!\n");
        return EXIT_FAILURE;
    }

    int num_sensores = (argc - 5) / 2;
    int total_leituras = num_sensores * LEITURAS_POR_SENSOR;

    printf("Gerando %d leituras para %d sensores...\n", total_leituras, num_sensores);

    Leitura *leituras = (Leitura *)malloc(total_leituras * sizeof(Leitura));
    if (leituras == NULL) {
        perror("Erro ao alocar memória");
        return EXIT_FAILURE;
    }

    int k = 0;
    for (int i = 0; i < num_sensores; i++) {
        const char *id_sensor = argv[5 + i * 2];
        const char *tipo_str = argv[6 + i * 2];
        TipoDado tipo = string_para_tipo(tipo_str);

        if (tipo == T_INVALIDO) {
            fprintf(stderr, "Aviso: Tipo '%s' inválido para sensor '%s'. Ignorando.\n", tipo_str, id_sensor);
            total_leituras -= LEITURAS_POR_SENSOR;
            continue;
        }

        const char *tipo_nome = "";
        switch (tipo) {
            case T_INT: tipo_nome = "CONJ_Z (inteiro)"; break;
            case T_FLOAT: tipo_nome = "CONJ_Q (float)"; break;
            case T_BOOL: tipo_nome = "BINARIO (booleano)"; break;
            case T_STRING: tipo_nome = "TEXTO (string)"; break;
            case T_INVALIDO: break;
        }

        printf("  - Sensor: %-10s Tipo: %-18s Leituras: %d\n", id_sensor, tipo_nome, LEITURAS_POR_SENSOR);

        for (int j = 0; j < LEITURAS_POR_SENSOR; j++) {
            double frac = (double)rand() / RAND_MAX;
            time_t random_ts = ts_inicio + (time_t)(frac * (ts_fim - ts_inicio));

            leituras[k].timestamp = random_ts;
            strncpy(leituras[k].sensor_id, id_sensor, MAX_ID_LEN - 1);
            leituras[k].sensor_id[MAX_ID_LEN - 1] = '\0';

            gerar_valor_aleatorio(leituras[k].valor, tipo);
            k++;
        }
    }

    printf("Embaralhando %d leituras...\n", k);
    embaralhar(leituras, k);

    const char *nome_arquivo = "dados_sensores.txt";
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao criar arquivo");
        free(leituras);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < k; i++) {
        fprintf(arquivo, "%ld %s %s\n",
                leituras[i].timestamp,
                leituras[i].sensor_id,
                leituras[i].valor);
    }
    fclose(arquivo);

    free(leituras);
    printf("Arquivo '%s' gerado com sucesso!\n", nome_arquivo);
    printf("Total de leituras: %d\n", k);

    return EXIT_SUCCESS;
}