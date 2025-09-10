/**
 * RECEPTOR DE SINAIS IR - Raspberry Pi Pico
 * Captura até 5 sinais IR e exibe dados no formato rawSignal[]
 * FORMATO: uint16_t rawSignal[] = {tempo1, tempo2, tempo3, ...}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"

// Bibliotecas do display (se disponível)
#ifdef USE_DISPLAY
#include "ssd1306.h"
#include "font.h"
#endif

// Configurações
#define IR_RX_PIN 17        // Sensor IR
#define LED_STATUS 25       // LED onboard
#define MAX_TRANSITIONS 1024 // Máximo de transições por sinal (aumentado)
#define MIN_SIGNAL_GAP_US 10000   // 10ms de silêncio = fim de comando (aumentado)
#define MAX_PULSE_US 50000        // Máximo 50ms por pulso (aumentado)
#define MIN_PULSE_US 50           // Mínimo 50us por pulso (diminuído)
#define MAX_SIGNALS 5             // Máximo de sinais para capturar
#define DEBOUNCE_TIME_US 20       // Tempo de debounce em microssegundos

// Estrutura do sinal capturado no formato RAW
typedef struct {
    uint16_t raw_data[MAX_TRANSITIONS];  // Tempos em microssegundos
    uint16_t count;                      // Número de tempos
    uint32_t total_duration_ms;          // Duração total
    bool is_complete;                    // Sinal completo?
    char name[32];                       // Nome do sinal
} ir_raw_signal_t;

// Variáveis globais
volatile ir_raw_signal_t current_signal;
volatile bool signal_ready = false;
volatile bool capturing = false;
volatile uint32_t last_transition_time = 0;
volatile uint32_t signal_start_time = 0;
volatile bool last_state = true; // Estado anterior do pino
volatile uint32_t transition_count = 0;
volatile uint32_t last_debounce_time = 0;

// Banco de sinais capturados
ir_raw_signal_t captured_signals[MAX_SIGNALS];
uint8_t signal_count = 0;

// Timer para detectar fim de sinal
repeating_timer_t signal_timer;

// Callback do timer para detectar fim de sinal
bool signal_timeout_callback(repeating_timer_t *rt) {
    if (capturing) {
        uint32_t now = to_us_since_boot(get_absolute_time());
        uint32_t silence_time = now - last_transition_time;
        
        // Se passou tempo suficiente sem transições, finaliza o sinal
        if (silence_time > MIN_SIGNAL_GAP_US && current_signal.count > 10) {
            current_signal.is_complete = true;
            current_signal.total_duration_ms = (now - signal_start_time) / 1000;
            signal_ready = true;
            capturing = false;
            gpio_put(LED_STATUS, 0);
            
            printf(">>> Sinal finalizado!\n");
            printf("Tempos: %d | Duração: %d ms | Silêncio: %d us\n", 
                   current_signal.count, current_signal.total_duration_ms, silence_time);
        }
    }
    return true;
}

// Callback de interrupção do IR - VERSÃO RAW
void ir_transition_callback(uint gpio, uint32_t events) {
    uint32_t now = to_us_since_boot(get_absolute_time());
    bool current_state = gpio_get(IR_RX_PIN);
    
    // Debounce simples
    if ((now - last_debounce_time) < DEBOUNCE_TIME_US) {
        return;
    }
    last_debounce_time = now;
    
    // Debug: mostra mudanças de estado
    if (transition_count < 20) {
        printf("GPIO=%d, State=%d, Time=%lu\n", gpio, current_state, now);
        transition_count++;
    }
    
    if (capturing) {
        // Calcula duração desde a última transição
        uint32_t duration = now - last_transition_time;
        
        // Verifica se a duração está dentro dos limites
        if (duration >= MIN_PULSE_US && duration <= MAX_PULSE_US) {
            if (current_signal.count < MAX_TRANSITIONS - 1) {
                // Armazena apenas o tempo (formato raw)
                current_signal.raw_data[current_signal.count] = (uint16_t)duration;
                current_signal.count++;
                
                // Debug dos primeiros tempos
                if (current_signal.count <= 20) {
                    printf("Tempo %d: %dus (%s->%s)\n", 
                           current_signal.count, 
                           duration,
                           last_state ? "HIGH" : "LOW",
                           current_state ? "HIGH" : "LOW");
                }
            } else {
                printf(">>> AVISO: Máximo de tempos atingido!\n");
                current_signal.is_complete = true;
                signal_ready = true;
                capturing = false;
                gpio_put(LED_STATUS, 0);
            }
        } else if (duration > MAX_PULSE_US) {
            printf(">>> AVISO: Pulso muito longo: %dus (max %dus)\n", duration, MAX_PULSE_US);
        }
    } else {
        // Detecta início de sinal - tradicionalmente LOW no receptor TSOP
        if (!current_state) { // Sinal IR detectado (LOW)
            printf("\n>>> NOVO SINAL IR DETECTADO!\n");
            printf("Estado inicial: %s\n", current_state ? "HIGH" : "LOW");
            
            capturing = true;
            signal_start_time = now;
            current_signal.count = 0;
            current_signal.is_complete = false;
            transition_count = 0;
            gpio_put(LED_STATUS, 1);
            
            printf("Iniciando captura...\n");
        }
    }
    
    // Atualiza estado e tempo
    last_state = current_state;
    last_transition_time = now;
}

// Testa o pino IR manualmente
void test_ir_pin(void) {
    printf("\n>>> TESTE DO PINO IR (%d)\n", IR_RX_PIN);
    printf("Monitore as mudanças por 10 segundos...\n");
    printf("Aponte o controle e pressione botões.\n");
    
    bool last_reading = gpio_get(IR_RX_PIN);
    printf("Estado inicial: %s\n", last_reading ? "HIGH" : "LOW");
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t change_count = 0;
    
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < 10000) {
        bool current_reading = gpio_get(IR_RX_PIN);
        
        if (current_reading != last_reading) {
            change_count++;
            printf("Mudança %lu: %s -> %s\n", 
                   change_count,
                   last_reading ? "HIGH" : "LOW", 
                   current_reading ? "HIGH" : "LOW");
            last_reading = current_reading;
            
            // Pisca LED a cada mudança
            gpio_put(LED_STATUS, !gpio_get(LED_STATUS));
        }
        
        sleep_ms(1);
    }
    
    printf("Teste concluído. Total de mudanças: %lu\n", change_count);
    gpio_put(LED_STATUS, 0);
    
    if (change_count == 0) {
        printf(">>> PROBLEMA: Nenhuma mudança detectada!\n");
        printf("Verifique:\n");
        printf("- Conexão do sensor IR no pino %d\n", IR_RX_PIN);
        printf("- Alimentação do sensor (3.3V)\n");
        printf("- Sensor funcionando\n");
    } else {
        printf(">>> Sensor respondendo normalmente!\n");
    }
}

// Exibe os dados do sinal no formato RAW para cópia direta
void print_raw_signal_data(const ir_raw_signal_t* signal, int signal_num) {
    printf("\n=====================================\n");
    printf("SINAL %d - FORMATO RAW PARA CÓPIA:\n", signal_num + 1);
    printf("=====================================\n");
    printf("// Sinal %d: %s\n", signal_num + 1, signal->name);
    printf("// Tempos: %d | Duração: %d ms\n", signal->count, signal->total_duration_ms);
    printf("// Primeira posição = ON, depois alterna ON/OFF/ON/OFF...\n");
    
    printf("uint16_t rawSignal%d[] = {\n", signal_num + 1);
    
    // Imprime os dados em linhas de 12 valores
    for (int i = 0; i < signal->count; i++) {
        if (i % 12 == 0) printf("    ");
        
        printf("%d", signal->raw_data[i]);
        
        if (i < signal->count - 1) printf(", ");
        
        if (i % 12 == 11 || i == signal->count - 1) {
            printf("\n");
        }
    }
    
    printf("};\n");
    printf("#define RAW_SIGNAL%d_LENGTH %d\n", signal_num + 1, signal->count);
    
    // Análise do sinal
    printf("\n// ANÁLISE:\n");
    printf("// - Total de tempos: %d\n", signal->count);
    printf("// - Duração total: %d ms\n", signal->total_duration_ms);
    printf("// - Primeiro tempo: %dus (ON)\n", signal->raw_data[0]);
    if (signal->count > 1) {
        printf("// - Último tempo: %dus (%s)\n", 
               signal->raw_data[signal->count-1],
               (signal->count % 2 == 1) ? "ON" : "OFF");
    }
    
    // Estatísticas básicas
    uint16_t min_time = signal->raw_data[0];
    uint16_t max_time = signal->raw_data[0];
    uint32_t sum_time = 0;
    
    for (int i = 0; i < signal->count; i++) {
        if (signal->raw_data[i] < min_time) min_time = signal->raw_data[i];
        if (signal->raw_data[i] > max_time) max_time = signal->raw_data[i];
        sum_time += signal->raw_data[i];
    }
    
    printf("// - Tempo mín: %dus, máx: %dus, médio: %dus\n", 
           min_time, max_time, sum_time / signal->count);
    
    printf("=====================================\n");
}

// Exibe todos os sinais no formato para transmissor
void print_all_raw_signals_data() {
    printf("\n\n###########################################\n");
    printf("TODOS OS SINAIS - FORMATO PARA TRANSMISSOR\n");
    printf("###########################################\n");
    
    // Imprime cada sinal individual
    for (int i = 0; i < signal_count; i++) {
        print_raw_signal_data(&captured_signals[i], i);
        printf("\n");
    }
    
    printf("\n// EXEMPLO DE USO NO TRANSMISSOR:\n");
    printf("// Substitua o rawSignal[] no seu código transmissor\n");
    printf("// por qualquer um dos arrays acima (rawSignal1[], rawSignal2[], etc.)\n\n");
    
    printf("// Para usar múltiplos sinais, você pode criar um array de ponteiros:\n");
    printf("uint16_t* all_raw_signals[%d] = {\n", signal_count);
    for (int i = 0; i < signal_count; i++) {
        printf("    rawSignal%d", i + 1);
        if (i < signal_count - 1) printf(",");
        printf("\n");
    }
    printf("};\n\n");
    
    printf("uint16_t signal_lengths[%d] = {\n", signal_count);
    for (int i = 0; i < signal_count; i++) {
        printf("    RAW_SIGNAL%d_LENGTH", i + 1);
        if (i < signal_count - 1) printf(",");
        printf("\n");
    }
    printf("};\n");
    
    printf("\n###########################################\n");
    printf("COPIE OS DADOS ACIMA PARA O TRANSMISSOR\n");
    printf("FORMATO COMPATÍVEL: uint16_t rawSignal[]\n");
    printf("###########################################\n");
}

// Processa comandos
void process_commands(void) {
    int c = getchar_timeout_us(1000);
    if (c != PICO_ERROR_TIMEOUT) {
        if (c == 't' || c == 'T') {
            test_ir_pin();
        } else if (c == 'r' || c == 'R') {
            printf(">>> Reset - reiniciando captura...\n");
            signal_count = 0;
            signal_ready = false;
            capturing = false;
            current_signal.count = 0;
            gpio_put(LED_STATUS, 0);
        } else if (c == 's' || c == 'S') {
            printf(">>> Estado atual:\n");
            printf("Sinais capturados: %d/%d\n", signal_count, MAX_SIGNALS);
            printf("Capturando: %s\n", capturing ? "SIM" : "NÃO");
            printf("Sinal pronto: %s\n", signal_ready ? "SIM" : "NÃO");
            printf("Estado do pino IR: %s\n", gpio_get(IR_RX_PIN) ? "HIGH" : "LOW");
            printf("Tempos no sinal atual: %d\n", current_signal.count);
        } else if (c == 'h' || c == 'H') {
            printf("\n>>> COMANDOS DISPONÍVEIS:\n");
            printf("t - Teste do pino IR\n");
            printf("r - Reset da captura\n");
            printf("s - Status atual\n");
            printf("h - Ajuda\n");
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(3000);
    
    printf("\n>>> RECEPTOR IR - FORMATO RAW uint16_t[]\n");
    printf("=========================================\n");
    printf(">> Receptor IR no pino %d\n", IR_RX_PIN);
    printf(">> LED de status no pino %d\n", LED_STATUS);
    printf(">> Captura até %d sinais\n", MAX_SIGNALS);
    printf(">> Formato: uint16_t rawSignal[]\n");
    printf("=========================================\n");
    
    // Configura hardware
    gpio_init(LED_STATUS);
    gpio_set_dir(LED_STATUS, GPIO_OUT);
    gpio_put(LED_STATUS, 0);
    
    gpio_init(IR_RX_PIN);
    gpio_set_dir(IR_RX_PIN, GPIO_IN);
    // Remove pulls - importante para sensores TSOP
    gpio_disable_pulls(IR_RX_PIN);
    
    // Configura interrupção para ambas as bordas
    gpio_set_irq_enabled_with_callback(IR_RX_PIN, 
                                      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 
                                      true, 
                                      &ir_transition_callback);
    
    // Timer para detectar fim de sinal (verifica a cada 5ms)
    add_repeating_timer_ms(5, signal_timeout_callback, NULL, &signal_timer);
    
    // Inicializa variáveis
    current_signal.count = 0;
    current_signal.is_complete = false;
    signal_ready = false;
    capturing = false;
    signal_count = 0;
    last_state = gpio_get(IR_RX_PIN);
    
    printf("\n>>> INSTRUÇÕES:\n");
    printf("1. Aponte o controle remoto para o sensor\n");
    printf("2. Pressione um botão por vez\n");
    printf("3. Aguarde a captura completa\n");
    printf("4. Repita até capturar %d sinais\n", MAX_SIGNALS);
    printf("5. Digite 't' para testar o sensor\n");
    printf("6. Digite 'h' para ver mais comandos\n");
    printf("=========================================\n");
    
    // Teste inicial do pino
    printf("\n>>> Estado inicial do pino IR: %s\n", gpio_get(IR_RX_PIN) ? "HIGH" : "LOW");
    
    printf("\n>>> Aguardando sinais IR... (%d/%d capturados)\n", signal_count, MAX_SIGNALS);
    printf(">>> Digite 't' para testar o sensor primeiro!\n");
    
    while (signal_count < MAX_SIGNALS) {
        // Processa comandos do usuário
        process_commands();
        
        // Processa sinal capturado
        if (signal_ready) {
            // Valida se o sinal tem dados suficientes
            if (current_signal.count < 10) {
                printf(">>> AVISO: Sinal muito curto (%d tempos), ignorando...\n", 
                       current_signal.count);
                signal_ready = false;
                current_signal.count = 0;
                continue;
            }
            
            // Cria nome para o sinal
            snprintf(captured_signals[signal_count].name, 32, "SINAL_RAW_%d", signal_count + 1);
            
            // Copia o sinal capturado
            captured_signals[signal_count] = *((ir_raw_signal_t*)&current_signal);
            
            printf("\n>>> SINAL %d CAPTURADO COM SUCESSO!\n", signal_count + 1);
            printf("Nome: %s\n", captured_signals[signal_count].name);
            printf("Tempos: %d\n", captured_signals[signal_count].count);
            printf("Duração: %d ms\n", captured_signals[signal_count].total_duration_ms);
            
            signal_count++;
            
            // Exibe dados do sinal atual no formato RAW
            print_raw_signal_data(&captured_signals[signal_count - 1], signal_count - 1);
            
            // Reset para próximo sinal
            signal_ready = false;
            current_signal.count = 0;
            current_signal.is_complete = false;
            
            if (signal_count < MAX_SIGNALS) {
                printf("\n>>> Aguardando próximo sinal... (%d/%d capturados)\n", 
                       signal_count, MAX_SIGNALS);
                printf(">>> Pressione outro botão do controle!\n");
            }
        }
        
        sleep_ms(10);
    }
    
    // Exibe todos os dados capturados no formato RAW
    printf("\n\n>>> CAPTURA CONCLUÍDA! %d sinais capturados.\n", signal_count);
    print_all_raw_signals_data();
    
    printf("\n>>> CAPTURA FINALIZADA!\n");
    printf("Agora copie os dados rawSignal[] e cole no código transmissor.\n");
    printf("Formato totalmente compatível com o transmissor!\n");
    
    // Pisca LED para indicar fim
    while (true) {
        gpio_put(LED_STATUS, 1);
        sleep_ms(250);
        gpio_put(LED_STATUS, 0);
        sleep_ms(250);
        
        // Continua processando comandos
        process_commands();
    }
    
    return 0;
}