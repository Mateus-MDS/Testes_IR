/**
 * EMISSOR DE SINAIS IR SIMPLES - Raspberry Pi Pico
 * Emite sinal IR a cada 7 segundos no pino 16
 * Formato: uint16_t rawSignal[] = {tempo1, tempo2, tempo3, ...}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Configurações
#define IR_TX_PIN 16        // LED IR transmissor
#define LED_STATUS 25       // LED onboard
#define TRANSMISSION_INTERVAL_MS 7000  // 7 segundos entre transmissões
#define IR_CARRIER_FREQ 38000           // Frequência do carrier IR

#define Botao 15

#define led_ext 1

bool Estado_arcondicionado = true;

void gpio_irq_handler(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Debouncing de 300ms (300000 microsegundos)
    if ((current_time - last_time) > 300000) {
        
        // Verifica se é o botão correto E se está pressionado (LOW devido ao pull-up)
        if (gpio == Botao && !gpio_get(Botao)) {
            last_time = current_time;
            
            Estado_arcondicionado = !Estado_arcondicionado;
            printf("Estado alterado: %s\n", Estado_arcondicionado ? "OFF" : "ON");
        }
    }
}

// SINAL IR NO FORMATO RAW (tempos em microssegundos)
// Primeira posição = ON, depois alterna ON/OFF/ON/OFF...
uint16_t rawSignal_off[] = {
    3603, 1758, 360, 1359, 404, 1362, 405, 344, 423, 352, 426, 348, 
    404, 1335, 429, 345, 427, 348, 413, 1338, 404, 1361, 404, 345, 
    427, 1312, 429, 345, 426, 348, 421, 1319, 428, 1335, 362, 426, 
    406, 1334, 403, 1333, 405, 345, 427, 347, 408, 1358, 403, 344, 
    407, 368, 390, 1361, 403, 373, 426, 349, 403, 372, 410, 364, 
    426, 349, 427, 347, 427, 348, 391, 423, 406, 345, 425, 349, 
    426, 348, 426, 349, 404, 370, 403, 372, 411, 364, 413, 401, 
    406, 343, 426, 349, 427, 348, 403, 371, 412, 1354, 403, 344, 
    426, 349, 415, 399, 405, 344, 403, 372, 403, 1338, 427, 1334, 
    404, 345, 404, 371, 414, 360, 416, 1336, 403, 1362, 404, 1333, 
    406, 343, 413, 363, 410, 364, 402, 373, 426, 349, 415, 399, 
    404, 345, 403, 372, 404, 1336, 427, 1335, 404, 1334, 404, 345, 
    403, 372, 415, 399, 405, 344, 403, 372, 403, 372, 402, 372, 
    402, 373, 402, 373, 402, 372, 416, 398, 405, 345, 427, 348, 
    425, 350, 426, 348, 427, 348, 427, 348, 428, 347, 415, 398, 
    381, 394, 381, 394, 380, 394, 380, 395, 380, 395, 378, 396, 
    378, 397, 398, 391, 352, 423, 352, 422, 352, 423, 376, 399, 
    379, 395, 383, 392, 383, 392, 399, 367, 405, 370, 404, 1331, 
    402, 1336, 401, 376, 401, 373, 401, 374, 399, 1337, 414
};

uint16_t rawSignal_on[] = {
    3585, 1762, 354, 1393, 411, 1328, 413, 342, 392, 383, 364, 411, 
    388, 1369, 409, 346, 365, 410, 445, 1326, 414, 1324, 412, 344, 
    389, 1368, 408, 347, 366, 409, 365, 1393, 408, 1329, 386, 384, 
    364, 1393, 410, 1329, 409, 346, 364, 411, 364, 1392, 386, 370, 
    365, 410, 444, 1327, 410, 346, 363, 412, 363, 412, 363, 411, 
    364, 410, 364, 411, 364, 411, 442, 347, 364, 410, 365, 410, 
    363, 411, 391, 384, 364, 411, 365, 410, 363, 411, 447, 342, 
    365, 410, 364, 1392, 409, 348, 364, 410, 390, 1366, 410, 347, 
    391, 384, 444, 1326, 411, 1327, 412, 344, 395, 380, 395, 1361, 
    410, 347, 394, 381, 395, 379, 446, 1324, 384, 1353, 391, 367, 
    399, 1356, 411, 347, 398, 377, 400, 374, 401, 374, 444, 344, 
    402, 1353, 414, 344, 403, 372, 403, 372, 429, 1325, 415, 345, 
    429, 349, 439, 362, 413, 361, 416, 368, 403, 372, 380, 394, 
    379, 396, 378, 397, 376, 399, 390, 377, 402, 369, 401, 398, 
    378, 374, 400, 374, 399, 375, 399, 375, 396, 380, 407, 381, 
    390, 384, 366, 409, 364, 411, 365, 409, 366, 409, 386, 388, 
    389, 386, 378, 411, 364, 411, 363, 412, 362, 412, 363, 412, 
    364, 410, 364, 411, 363, 412, 375, 1396, 343, 413, 360, 414, 
    361, 1396, 343, 1396, 342, 1396, 340, 1398, 340, 416, 368
};

uint16_t temp_para_22[] = {
    3609, 1760, 381, 1338, 403, 1363, 404, 344, 404, 371, 403, 372, 
    404, 1336, 427, 345, 403, 372, 415, 1335, 404, 1362, 405, 344, 
    404, 1360, 404, 344, 404, 371, 403, 1362, 403, 1334, 389, 399, 
    405, 1313, 425, 1334, 405, 344, 403, 372, 403, 1361, 404, 344, 
    403, 372, 419, 1334, 428, 346, 402, 372, 403, 372, 403, 372, 
    403, 372, 402, 372, 403, 372, 419, 372, 427, 345, 403, 372, 
    402, 372, 403, 372, 403, 372, 402, 372, 403, 373, 419, 370, 
    428, 345, 404, 1361, 404, 344, 403, 372, 423, 1341, 404, 346, 
    428, 318, 444, 1338, 428, 1334, 403, 370, 381, 394, 380, 1333, 
    405, 1333, 403, 397, 354, 421, 361, 1365, 400, 400, 353, 422, 
    377, 1336, 399, 401, 382, 393, 381, 394, 383, 392, 393, 371, 
    405, 1331, 404, 373, 403, 372, 402, 1333, 403, 374, 400, 375, 
    398, 376, 410, 379, 396, 378, 394, 381, 392, 383, 389, 385, 
    391, 384, 391, 384, 392, 382, 379, 410, 390, 385, 364, 411, 
    363, 411, 363, 412, 365, 410, 389, 386, 362, 413, 375, 414, 
    360, 414, 362, 414, 361, 414, 359, 416, 358, 435, 340, 435, 
    340, 438, 350, 436, 338, 437, 337, 437, 337, 438, 337, 438, 
    336, 439, 335, 440, 335, 482, 306, 1404, 333, 1406, 331, 1432, 
    306, 445, 329, 470, 305, 445, 330, 469, 305, 1444, 303
};

uint16_t temp_para_20[] = {
    3611, 1759, 364, 1356, 428, 1314, 428, 344, 427, 317, 461, 300, 
    474, 1308, 430, 345, 427, 349, 421, 1327, 405, 1339, 428, 344, 
    412, 1326, 428, 346, 427, 348, 427, 1311, 430, 1312, 386, 424, 
    406, 1308, 429, 1311, 428, 344, 403, 371, 427, 1312, 429, 345, 
    410, 364, 417, 1334, 404, 373, 422, 352, 427, 348, 428, 347, 
    426, 349, 427, 347, 427, 348, 392, 422, 405, 345, 428, 346, 
    428, 346, 427, 348, 427, 349, 426, 347, 410, 365, 417, 397, 
    406, 344, 428, 1311, 429, 344, 403, 372, 427, 1311, 429, 345, 
    403, 373, 415, 1336, 429, 1336, 403, 345, 404, 372, 404, 1337, 
    426, 1334, 404, 346, 404, 396, 399, 1325, 429, 1311, 429, 371, 
    377, 1335, 405, 395, 353, 422, 351, 423, 352, 424, 400, 388, 
    383, 1329, 400, 401, 385, 389, 386, 1326, 402, 400, 385, 389, 
    409, 345, 446, 341, 403, 370, 406, 370, 404, 369, 404, 371, 
    402, 373, 401, 373, 401, 373, 443, 349, 396, 376, 395, 380, 
    393, 382, 390, 384, 391, 384, 392, 383, 392, 383, 379, 430, 
    370, 384, 364, 410, 362, 413, 363, 412, 367, 408, 389, 386, 
    361, 414, 378, 430, 342, 433, 342, 415, 360, 433, 340, 434, 
    340, 435, 339, 435, 340, 438, 350, 1399, 338, 438, 337, 437, 
    337, 1401, 337, 439, 335, 440, 335, 440, 334, 1439, 307
};

uint16_t fan_1[] = {
    3610, 1735, 437, 1310, 427, 1336, 404, 345, 402, 372, 404, 371, 
    402, 1363, 403, 345, 402, 373, 443, 1335, 404, 1333, 404, 346, 
    402, 1361, 405, 344, 403, 372, 403, 1360, 405, 1334, 391, 372, 
    402, 1361, 405, 1332, 406, 345, 402, 372, 402, 1359, 407, 345, 
    402, 373, 443, 1331, 407, 346, 402, 372, 402, 373, 401, 373, 
    402, 373, 402, 373, 402, 373, 442, 347, 401, 373, 401, 373, 
    402, 374, 400, 374, 402, 373, 400, 374, 401, 374, 443, 346, 
    400, 375, 401, 1356, 410, 345, 402, 373, 401, 1357, 382, 373, 
    400, 374, 443, 1329, 383, 1355, 384, 371, 401, 374, 400, 1357, 
    384, 1354, 409, 347, 400, 374, 441, 1331, 409, 1329, 409, 346, 
    401, 1357, 408, 347, 401, 374, 401, 374, 400, 374, 443, 1329, 
    382, 373, 400, 1357, 408, 347, 402, 1357, 408, 347, 401, 373, 
    401, 373, 443, 347, 401, 373, 402, 373, 401, 374, 401, 373, 
    402, 373, 401, 374, 401, 373, 443, 346, 402, 373, 401, 373, 
    403, 372, 402, 372, 402, 373, 402, 373, 402, 372, 443, 346, 
    402, 373, 402, 372, 402, 373, 402, 373, 402, 372, 403, 372, 
    403, 372, 443, 346, 401, 373, 402, 373, 402, 372, 406, 371, 
    427, 349, 427, 346, 428, 371, 389, 399, 405, 370, 381, 1332, 
    405, 1333, 404, 396, 378, 397, 375, 399, 374, 1339, 385
};

uint16_t fan_2[] = {
    278, 134, 3609, 1760, 411, 1335, 405, 1332, 406, 345, 402, 372, 
    402, 373, 403, 1359, 406, 345, 402, 373, 443, 1332, 406, 1332, 
    407, 344, 403, 1359, 406, 345, 402, 373, 402, 1358, 408, 1329, 
    395, 373, 402, 1357, 408, 1330, 408, 345, 403, 372, 402, 1357, 
    409, 345, 402, 373, 442, 1330, 409, 345, 402, 373, 402, 373, 
    402, 372, 402, 373, 402, 373, 402, 373, 442, 346, 402, 373, 
    402, 372, 403, 372, 403, 371, 403, 372, 403, 372, 402, 373, 
    443, 345, 402, 373, 402, 373, 402, 372, 403, 372, 403, 1353, 
    410, 346, 403, 373, 443, 1326, 411, 1326, 412, 346, 403, 372, 
    403, 1352, 411, 1327, 412, 346, 403, 370, 445, 1326, 411, 1328, 
    411, 350, 426, 1327, 410, 371, 404, 370, 404, 370, 382, 393, 
    390, 1357, 384, 396, 378, 1355, 380, 400, 375, 1358, 383, 396, 
    378, 375, 400, 373, 416, 373, 402, 373, 400, 375, 398, 376, 
    397, 377, 395, 381, 392, 382, 391, 383, 410, 379, 366, 409, 
    365, 409, 390, 385, 391, 384, 390, 385, 389, 385, 388, 387, 
    378, 411, 363, 411, 363, 412, 364, 413, 386, 386, 388, 387, 
    362, 413, 360, 415, 377, 411, 362, 413, 362, 413, 361, 414, 
    360, 416, 358, 417, 358, 435, 340, 438, 350, 436, 338, 437, 
    337, 438, 337, 1400, 338, 437, 337, 438, 336, 439, 335, 1438, 
    309
};

// Definir tamanhos dos arrays
#define RAW_SIGNAL_OFF_LENGTH (sizeof(rawSignal_off) / sizeof(rawSignal_off[0]))
#define RAW_SIGNAL_ON_LENGTH (sizeof(rawSignal_on) / sizeof(rawSignal_on[0]))

// Variáveis globais
uint32_t transmission_counter = 0;

// Transmite sinal IR com carrier 38kHz usando formato raw
void transmit_raw_ir_signal() {
    // Seleciona qual sinal usar baseado no estado
    uint16_t* current_signal;
    int signal_length;
    
    if (Estado_arcondicionado) {
        current_signal = rawSignal_off;
        signal_length = RAW_SIGNAL_OFF_LENGTH;
        printf(">>> TRANSMITINDO SINAL IR OFF (#%lu)\n", ++transmission_counter);
    } else {
        current_signal = rawSignal_on;
        signal_length = RAW_SIGNAL_ON_LENGTH;
        printf(">>> TRANSMITINDO SINAL IR ON (#%lu)\n", ++transmission_counter);
    }
    
    printf("Total de tempos: %d\n", signal_length);
    
    // Liga LED de status
    gpio_put(LED_STATUS, 1);
    
    // Calcula período do carrier 38kHz
    uint32_t half_period_us = 1000000 / (IR_CARRIER_FREQ * 2);  // ~13.16 us
    
    // Processa cada tempo do array
    for (int i = 0; i < signal_length; i++) {
        uint32_t duration_us = current_signal[i];  // Usa o sinal correto
        bool is_on_period = (i % 2 == 0);  // Posições pares = ON, ímpares = OFF
        
        if (is_on_period) {
            // Período ON: Gera carrier 38kHz
            absolute_time_t start_time = get_absolute_time();
            
            while (absolute_time_diff_us(start_time, get_absolute_time()) < duration_us) {
                gpio_put(IR_TX_PIN, 1);
                busy_wait_us_32(half_period_us);
                gpio_put(IR_TX_PIN, 0);
                busy_wait_us_32(half_period_us);
            }
        } else {
            // Período OFF: Silêncio (sem carrier)
            gpio_put(IR_TX_PIN, 0);
            busy_wait_us_32(duration_us);
        }
    }
    
    // Garante que terminou em LOW
    gpio_put(IR_TX_PIN, 0);
    gpio_put(LED_STATUS, 0);
    
    printf(">>> Transmissão concluída!\n\n");
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Aguarda inicialização da serial
    
    printf("\n=== EMISSOR IR AUTOMÁTICO (RAW FORMAT) ===\n");
    printf("Pino IR: %d\n", IR_TX_PIN);
    printf("Carrier: %d Hz\n", IR_CARRIER_FREQ);
    printf("Intervalo: %d segundos\n", TRANSMISSION_INTERVAL_MS / 1000);
    printf("Sinal OFF: %d tempos\n", RAW_SIGNAL_OFF_LENGTH);
    printf("Sinal ON: %d tempos\n", RAW_SIGNAL_ON_LENGTH);
    printf("=========================================\n\n");
    
    // Configura hardware
    gpio_init(LED_STATUS);
    gpio_set_dir(LED_STATUS, GPIO_OUT);
    gpio_put(LED_STATUS, 0);

    gpio_init(led_ext);
    gpio_set_dir(led_ext, GPIO_OUT);
    
    gpio_init(IR_TX_PIN);
    gpio_set_dir(IR_TX_PIN, GPIO_OUT);
    gpio_put(IR_TX_PIN, 0);

    gpio_init(Botao);
    gpio_set_dir(Botao, GPIO_IN);  // Corrigido: botão é INPUT
    gpio_pull_up(Botao);
    gpio_set_irq_enabled_with_callback(Botao, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    printf("Sistema iniciado! Transmitindo a cada %d segundos...\n", 
           TRANSMISSION_INTERVAL_MS / 1000);
    printf("Estado inicial: %s\n\n", Estado_arcondicionado ? "OFF" : "ON");
    
    absolute_time_t last_transmission = get_absolute_time();
    
    int numero = 0;
    // Loop principal

    while (true) {
        absolute_time_t now = get_absolute_time();
        int64_t elapsed_ms = absolute_time_diff_us(last_transmission, now) / 1000;
        
        if (elapsed_ms >= TRANSMISSION_INTERVAL_MS) {
            // Transmite o sinal baseado no estado atual
            transmit_raw_ir_signal();
            
            // Atualiza timestamp
            last_transmission = get_absolute_time();
        }

        if (Estado_arcondicionado){
            gpio_put(led_ext, true);
        } else{
            gpio_put(led_ext, false);
        }
        
        // Pequeno delay para não sobrecarregar o processador
        sleep_ms(100);
        numero += 1;
        printf("numero: %d\n", numero);
    }
    
    return 0;
}