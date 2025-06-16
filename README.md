# prova16-06-25
fé que deu bom


Este projeto contém três programas para gerenciar dados de sensores:

programa3: Gerador de dados de teste

programa1: Organizador de leituras de sensores

programa2: Consulta de leituras por timestamp

Pré-requisitos:
- Compilador GCC

- Linux/macOS ou Windows com WSL/Mingw

  Compilação(em ordem):
gcc -o programa3 programa3.c
gcc -o programa1 programa1.c
gcc -o programa2 programa2.c -lm

Como Executar
1. Gerar dados de teste = ./programa3 <data_inicio> <hora_inicio> <data_fim> <hora_fim> <sensor1> <tipo1> [sensor2 tipo2...] Exemplo: ./programa3 01-06-2025 08:00:00 01-06-2025 18:00:00 TEMP CONJ_Q PRESSAO CONJ_Z

2. Processar dados = ./programa1 dados_sensores.txt

3. Consultar leitura = ./programa2 <nome_sensor> <timestamp>. Exemplo: ./programa2 TEMP 1654012800

Espero que dê bom, no meus testes funcionram pelo menos
