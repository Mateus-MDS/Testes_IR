/**
 * Transmissor IR Universal - Protocolo NEC
 * Controle simples via Serial
 * 
 * Comandos:
 *   list                - Lista todos os comandos
 *   send <nome>         - Envia comando (ex: send KEY_POWER)
 *   raw <dev> <func>    - Envia valores diretos em hex (ex: raw 80 123)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "nec_transmit.h"

// Configuração
#define IR_TX_PIN 16

// Estrutura de comandos
typedef struct {
    const char *name;
    const char *protocol_code;
    uint8_t device;
    uint16_t function;  // ? Corrigido para uint16_t
} ir_command_t;

// Tabela de comandos
static const ir_command_t commands[] = {
    {"KEY_8",           "NEC80-14",  0x80, 0x14},
    {"KEY_9",           "NEC80-15",  0x80, 0x15},
    {"PRENSAR",         "NEC80-16",  0x80, 0x16},
    {"KEY_INFO",        "NEC80-17",  0x80, 0x17},
    {"SEIVA",           "NEC80-18",  0x80, 0x18},
    {"TVAV",            "NEC80-19",  0x80, 0x19},
    {"ULTIMA",          "NEC80-110", 0x80, 0x110},
    {"KEY_MUTE",        "NEC80-111", 0x80, 0x111},
    {"KEY_0",           "NEC80-112", 0x80, 0x112},
    {"KEY_1",           "NEC80-113", 0x80, 0x113},
    {"KEY_2",           "NEC80-114", 0x80, 0x114},
    {"KEY_3",           "NEC80-115", 0x80, 0x115},
    {"EXPOSICAO",       "NEC80-116", 0x80, 0x116},
    {"TEMPORIZADOR",    "NEC80-117", 0x80, 0x117},
    {"KEY_VOLUMEUP",    "NEC80-118", 0x80, 0x118},
    {"PREF",            "NEC80-119", 0x80, 0x119},
    {"KEY_VOLUMEDOWN",  "NEC80-121", 0x80, 0x121},
    {"KEY_POWER",       "NEC80-123", 0x80, 0x123},
    {"KEY_CHANNELDOWN", "NEC80-124", 0x80, 0x124},
    {"KEY_CHANNELUP",   "NEC80-125", 0x80, 0x125},
    {"KEY_4",           "NEC80-128", 0x80, 0x128},
    {"KEY_5",           "NEC80-129", 0x80, 0x129},
    {"KEY_6",           "NEC80-130", 0x80, 0x130},
    {"KEY_7",           "NEC80-131", 0x80, 0x131},
    {"MAGIA",           "NEC80-191", 0x80, 0x191},
    {"KEY_MENU",        "NEC80-194", 0x80, 0x194},
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(ir_command_t))

// Variáveis globais
static PIO pio;
static int tx_sm;

// Envia comando IR
void send_ir(uint8_t device, uint16_t function) {
    // Para NEC extendido (16 bits), envia os 8 bits baixos
    uint8_t func_low = (uint8_t)(function & 0xFF);
    
    uint32_t frame = nec_encode_frame(device, func_low);
    
    // Habilita o state machine
    pio_sm_set_enabled(pio, tx_sm, true);
    
    // Envia o frame
    pio_sm_put_blocking(pio, tx_sm, frame);
    
    // Aguarda completar a transmissão (~68ms para NEC completo)
    sleep_ms(70);
    
    // Desabilita o state machine para parar o sinal
    pio_sm_set_enabled(pio, tx_sm, false);
    
    printf("? Enviado: Device=0x%02X, Function=0x%03X\n", device, function);
}

// Busca comando por nome
const ir_command_t* find_command(const char *name) {
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcasecmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

// Lista comandos
void list_commands() {
    printf("\n=== Comandos Disponíveis ===\n");
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        printf("%2zu. %-18s %s\n", i+1, 
               commands[i].name, 
               commands[i].protocol_code);
    }
    printf("\n");
}

// Processa comando serial
void process_line(char *line) {
    printf("[DEBUG: process_line entrada='%s', len=%d]\n", line, strlen(line));
    
    // Remove newline
    char *nl = strchr(line, '\n');
    if (nl) *nl = '\0';
    
    // Ignora linha vazia
    if (strlen(line) == 0) {
        printf("[DEBUG: linha vazia]\n");
        return;
    }
    
    char *cmd = strtok(line, " ");
    printf("[DEBUG: comando='%s']\n", cmd);
    
    if (strcasecmp(cmd, "list") == 0) {
        list_commands();
        
    } else if (strcasecmp(cmd, "send") == 0) {
        char *name = strtok(NULL, " ");
        printf("[DEBUG: send name='%s']\n", name ? name : "NULL");
        if (name) {
            const ir_command_t *c = find_command(name);
            if (c) {
                printf("Enviando: %s (%s)\n", c->name, c->protocol_code);
                send_ir(c->device, c->function);
            } else {
                printf("? Comando não encontrado: %s\n", name);
            }
        } else {
            printf("? Uso: send <nome>\n");
        }
        
    } else if (strcasecmp(cmd, "raw") == 0) {
        char *dev = strtok(NULL, " ");
        char *func = strtok(NULL, " ");
        printf("[DEBUG: raw dev='%s' func='%s']\n", 
               dev ? dev : "NULL", func ? func : "NULL");
        if (dev && func) {
            uint8_t device = (uint8_t)strtol(dev, NULL, 16);
            uint16_t function = (uint16_t)strtol(func, NULL, 16);
            send_ir(device, function);
        } else {
            printf("? Uso: raw <device> <function> (hex)\n");
        }
        
    } else {
        printf("? Comando desconhecido. Use: list, send, raw\n");
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    printf("\n??????????????????????????????????????\n");
    printf("?  Transmissor IR - Protocolo NEC   ?\n");
    printf("?  Raspberry Pi Pico                 ?\n");
    printf("??????????????????????????????????????\n\n");

    // Inicializa PIO
    pio = pio0;
    tx_sm = nec_tx_init(pio, IR_TX_PIN);

    if (tx_sm == -1) {
        printf("? ERRO: Falha ao configurar PIO\n");
        return -1;
    }
    
    // Desabilita o state machine inicialmente
    pio_sm_set_enabled(pio, tx_sm, false);

    printf("? PIO configurado (GPIO %d)\n", IR_TX_PIN);
    printf("? %zu comandos carregados\n\n", NUM_COMMANDS);
    printf("Comandos:\n");
    printf("  list             - Lista comandos\n");
    printf("  send <nome>      - Envia comando\n");
    printf("  raw <dev> <func> - Valores diretos\n");
    printf("\nAtalhos rápidos:\n");
    printf("  L - list\n");
    printf("  P - send KEY_POWER\n");
    printf("  + - send KEY_VOLUMEUP\n");
    printf("  - - send KEY_VOLUMEDOWN\n");
    printf("  ^ - send KEY_CHANNELUP\n");
    printf("  v - send KEY_CHANNELDOWN\n");
    printf("\nUse ';' no fim se Enter não funcionar (ex: list;)\n");
    printf("> ");

    // Buffer de entrada
    char buffer[128];
    int pos = 0;

    while (true) {
        int c = getchar_timeout_us(0);
        
        if (c != PICO_ERROR_TIMEOUT) {
            // DEBUG: Mostra o código do caractere recebido
            printf("[DEBUG: recebido char=%d '%c']\n", c, c);
            
            // Aceita CR (13), LF (10) ou ';' como fim de comando
            if (c == '\r' || c == '\n' || c == ';') {
                printf("\n");
                buffer[pos] = '\0';
                printf("[DEBUG: buffer='%s', pos=%d]\n", buffer, pos);
                if (pos > 0) {
                    process_line(buffer);
                    pos = 0;
                }
                printf("> ");
            } 
            else if (c == 127 || c == 8) {  // Backspace
                if (pos > 0) {
                    pos--;
                    printf("\b \b");
                }
            } 
            else if (pos < sizeof(buffer) - 1) {
                buffer[pos++] = c;
                printf("%c", c);
                
                // ? Comandos numéricos simples (1 tecla)
                if (pos == 1 && c >= '0' && c <= '9') {
                    int num = c - '0';
                    bool handled = true;
                    
                    switch(num) {
                        case 0:
                            printf(" [POWER]\n");
                            send_ir(0x80, 0x123);
                            break;
                        case 1:
                            printf(" [VOL+]\n");
                            send_ir(0x80, 0x118);
                            break;
                        case 2:
                            printf(" [VOL-]\n");
                            send_ir(0x80, 0x121);
                            break;
                        case 3:
                            printf(" [CH+]\n");
                            send_ir(0x80, 0x125);
                            break;
                        case 4:
                            printf(" [CH-]\n");
                            send_ir(0x80, 0x124);
                            break;
                        case 5:
                            printf(" [MUTE]\n");
                            send_ir(0x80, 0x111);
                            break;
                        case 6:
                            printf(" [MENU]\n");
                            send_ir(0x80, 0x194);
                            break;
                        case 7:
                            printf(" [INFO]\n");
                            send_ir(0x80, 0x17);
                            break;
                        case 8:
                            printf(" [Tecla 0]\n");
                            send_ir(0x80, 0x112);
                            break;
                        case 9:
                            printf(" [Tecla 1]\n");
                            send_ir(0x80, 0x113);
                            break;
                        default:
                            handled = false;
                            break;
                    }
                    
                    if (handled) {
                        pos = 0;
                        printf("> ");
                    }
                }
                
                // Auto-processa "list" completo
                else if (pos == 4 && strncasecmp(buffer, "list", 4) == 0) {
                    printf(" [auto-processando]\n");
                    buffer[pos] = '\0';
                    process_line(buffer);
                    pos = 0;
                    printf("> ");
                }
            }
        }
        
        sleep_ms(10);
    }

    return 0;
}