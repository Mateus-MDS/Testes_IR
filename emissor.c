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
        
        // Pequeno delay para não sobrecarregar o processador
        sleep_ms(100);
    }
    
    return 0;
}