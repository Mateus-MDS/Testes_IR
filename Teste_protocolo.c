/**
 * Exemplo de controle de ar condicionado via PWM
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "custom_ir.h"

// Configurações
#define IR_PIN 16          // Pino para saída IR
#define LED_PIN 25        // LED onboard do Pico
#define BUTTON_PIN 17     // Pino do botão para teste

int current_state_novo = 20;

// Estados do sistema
typedef enum {
    STATE_OFF,
    State_on,
    Temp_20,
    Temp_22,
    STATE_MAX
} system_state_t;

static system_state_t current_state = STATE_OFF;

// Callback para botão
void button_callback(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Debounce simples (300ms)
    if (current_time - last_time < 300) {
        return;
    }
    last_time = current_time;
    
    // Avança para o próximo estado
    current_state = (current_state + 1) % STATE_MAX;
    
}

void comando_ir(){
    // Executa ação baseada no estado
    if (current_state_novo != current_state){

        current_state_novo = current_state;

        switch (current_state) {
        case STATE_OFF:
            printf("Estado: DESLIGADO\n");
            gpio_put(LED_PIN, 0);
            turn_off_ac();
            break;
            
        case State_on:
            printf("Estado: LIGADO\n");
            gpio_put(LED_PIN, 1);
            turn_on_ac();
            break;
            
        case Temp_20:
            printf("Estado: LIGADO - 20°C\n");
            gpio_put(LED_PIN, 1);
            set_temp_20c();
            break;

        case Temp_22:
            printf("Estado: LIGADO - 22°C\n");
            gpio_put(LED_PIN, 1);
            set_temp_22c();
            break;
            
        default:
            break;
    }
    }

}
// Configuração do botão
void setup_button() {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // Configurar interrupção
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
}

// Menu de comandos via UART
void show_menu() {
    printf("\n=== CONTROLE IR - AR CONDICIONADO ===\n");
    printf("1 - Ligar AC\n");
    printf("2 - Desligar AC\n");
    printf("3 - Temperatura 22°C\n");
    printf("4 - Temperatura 20°C\n");
    printf("5 - Ventilador Nível 1\n");
    printf("6 - Ventilador Nível 2\n");
    printf("7 - Mostrar estado atual\n");
    printf("0 - Mostrar menu\n");
    printf("====================================\n");
    printf("Digite uma opção: ");
}

// Processa comandos do teclado
void process_uart_input() {
    int ch = getchar_timeout_us(0);
    if (ch == PICO_ERROR_TIMEOUT) {
        return;
    }
    
    printf("%c\n", ch);
    
    switch (ch) {
        case '1':
            printf("Enviando comando: LIGAR\n");
            turn_on_ac();
            current_state = State_on;
            gpio_put(LED_PIN, 1);
            break;
            
        case '2':
            printf("Enviando comando: DESLIGAR\n");
            turn_off_ac();
            current_state = STATE_OFF;
            gpio_put(LED_PIN, 0);
            break;
            
        case '3':
            printf("Enviando comando: TEMPERATURA 22°C\n");
            set_temp_22c();
            break;
            
        case '4':
            printf("Enviando comando: TEMPERATURA 20°C\n");
            set_temp_20c();
            break;
            
        case '5':
            printf("Enviando comando: VENTILADOR NÍVEL 1\n");
            set_fan_level_1();
            break;
            
        case '6':
            printf("Enviando comando: VENTILADOR NÍVEL 2\n");
            set_fan_level_2();
            break;
            
        case '7':
            printf("Iniciando demonstração automática...\n");
            ir_demo();
            break;
            
        case '8':
            printf("Estado atual: ");
            switch (current_state) {
                case STATE_OFF:
                    printf("DESLIGADO\n");
                    break;
                case State_on:
                    printf("LIGADO - 22°C - Fan 1\n");
                    break;
                case Temp_20:
                    printf("LIGADO - 20°C - Fan 2\n");
                    break;
                default:
                    printf("DESCONHECIDO\n");
                    break;
            }
            break;
            
        case '0':
            show_menu();
            break;
            
        default:
            printf("Opção inválida! Digite '0' para ver o menu.\n");
            break;
    }
}

int main() {
    // Inicializar stdio
    stdio_init_all();
    
    // Aguardar conexão USB (opcional)
    sleep_ms(2000);
    
    printf("\n\n=== SISTEMA IR PARA AR CONDICIONADO ===\n");
    printf("Raspberry Pi Pico - Protocolo Customizado\n\n");
    
    // Configurar LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    
    // Configurar botão
    setup_button();
    printf("Botão configurado no pino %d\n", BUTTON_PIN);
    
    // Inicializar sistema IR
    if (!custom_ir_init(IR_PIN)) {
        printf("ERRO: Falha ao inicializar sistema IR!\n");
        while (1) {
            gpio_put(LED_PIN, 1);
            sleep_ms(100);
            gpio_put(LED_PIN, 0);
            sleep_ms(100);
        }
    }
    
    printf("Sistema IR inicializado no pino %d\n", IR_PIN);
    printf("Pronto para uso!\n");
    
    // Mostrar menu inicial
    show_menu();

    custom_ir_init(IR_PIN);
    
    // Loop principal
    while (1) {
        // Processar comandos UART
        process_uart_input();

        // Comandos a depender do botão
        comando_ir();
        
        // Pequena pausa para não sobrecarregar o sistema
        sleep_ms(10);
    }
    
    return 0;
}