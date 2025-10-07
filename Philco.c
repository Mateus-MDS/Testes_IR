/**
 * Transmissor IR Universal com Protocolo NEC
 * Formato: NEC<dispositivo>-<função>
 * Exemplo: NEC80-14 = dispositivo 0x80, função 0x14
 * 
 * Compatível com a biblioteca nec_transmit que usa:
 * - Portadora de 38.222 kHz
 * - Timing base de 562.5µs
 * - Dois state machines PIO (carrier_burst + carrier_control)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "nec_transmit.h"
#include "nec_receive.h"

// Configuração de pinos
#define IR_TX_PIN 16    // GPIO para LED IR (com resistor ~1.5k?)
#define IR_RX_PIN 15    // GPIO para receptor IR (VS1838B ou similar)

// Estrutura para mapear comandos
typedef struct {
    const char *name;
    const char *protocol_code;
    uint8_t device;
    uint16_t function;
} ir_command_t;

// Tabela de comandos (baseado na sua lista)
static const ir_command_t command_table[] = {
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

#define NUM_COMMANDS (sizeof(command_table) / sizeof(ir_command_t))

// Variáveis globais PIO
static PIO pio;
static int tx_sm;
static int rx_sm;

/**
 * Envia comando IR usando o protocolo NEC
 * 
 * O frame é codificado como:
 * - address (8 bits)
 * - ~address invertido (8 bits)
 * - data/function (8 bits)  
 * - ~data invertido (8 bits)
 * 
 * A biblioteca cuida automaticamente da modulação 38.222kHz
 * e do timing NEC padrão (562.5µs base)
 */
void send_ir_command_NEC(uint8_t device, uint8_t function) {
    // nec_encode_frame já faz: address | (~address << 8) | data << 16 | (~data << 24)
    uint32_t frame = nec_encode_frame(device, function);
    
    // Envia para o state machine PIO
    // A biblioteca usa 2 state machines:
    // - carrier_control_sm (retornado por nec_tx_init) recebe o frame
    // - carrier_burst_sm (interno) gera a portadora 38.222kHz
    pio_sm_put(pio, tx_sm, frame);
    
    printf("? Enviado: Device=0x%02X, Function=0x%02X (Frame=0x%08X)\n", 
           device, function, frame);
}

/**
 * Parse do protocolo: NEC<device>-<function>
 * Exemplo: "NEC80-14" -> device=0x80, function=0x14
 */
bool parse_protocol(const char *protocol, uint8_t *device, uint8_t *function) {
    if (strncmp(protocol, "NEC", 3) != 0) {
        return false;
    }
    
    // Encontra o hífen
    const char *dash = strchr(protocol + 3, '-');
    if (!dash) {
        return false;
    }
    
    // Parse device (hexadecimal)
    char device_str[10];
    int device_len = dash - (protocol + 3);
    strncpy(device_str, protocol + 3, device_len);
    device_str[device_len] = '\0';
    *device = (uint8_t)strtol(device_str, NULL, 16);
    
    // Parse function (decimal ou hexadecimal)
    *function = (uint8_t)strtol(dash + 1, NULL, 0);
    
    return true;
}

/**
 * Busca comando por nome
 */
const ir_command_t* find_command_by_name(const char *name) {
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcasecmp(command_table[i].name, name) == 0) {
            return &command_table[i];
        }
    }
    return NULL;
}

/**
 * Envia comando por string de protocolo
 */
bool send_by_protocol(const char *protocol) {
    uint8_t device, function;
    if (parse_protocol(protocol, &device, &function)) {
        printf("Enviando protocolo: %s\n", protocol);
        send_ir_command_NEC(device, function);
        return true;
    }
    return false;
}

/**
 * Envia comando por nome
 */
bool send_by_name(const char *name) {
    const ir_command_t *cmd = find_command_by_name(name);
    if (cmd) {
        printf("Enviando comando: %s (%s)\n", cmd->name, cmd->protocol_code);
        send_ir_command_NEC(cmd->device, cmd->function);
        return true;
    }
    return false;
}

/**
 * Lista todos os comandos disponíveis
 */
void list_commands() {
    printf("\n=== Comandos Disponíveis ===\n");
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        printf("%2zu. %-18s %s\n", i+1, 
               command_table[i].name, 
               command_table[i].protocol_code);
    }
    printf("\n");
}

/**
 * Menu de ajuda
 */
void show_help() {
    printf("\n=== Transmissor IR - Protocolo NEC ===\n");
    printf("Comandos:\n");
    printf("  list                  - Lista todos os comandos\n");
    printf("  send <nome>           - Envia comando por nome\n");
    printf("  protocol <NECxx-yy>   - Envia por código de protocolo\n");
    printf("  raw <device> <func>   - Envia valores diretos (hex)\n");
    printf("  help                  - Mostra esta ajuda\n");
    printf("\nExemplos:\n");
    printf("  send KEY_POWER\n");
    printf("  protocol NEC80-123\n");
    printf("  raw 80 14\n");
    printf("\n> ");
}

/**
 * Processa linha de comando
 */
void process_command(char *line) {
    // Remove newline
    char *newline = strchr(line, '\n');
    if (newline) *newline = '\0';
    
    // Remove espaços iniciais
    while (*line == ' ') line++;
    
    if (strlen(line) == 0) {
        printf("> ");
        return;
    }
    
    // Parse comando
    char *cmd = strtok(line, " ");
    
    if (strcasecmp(cmd, "list") == 0) {
        list_commands();
        
    } else if (strcasecmp(cmd, "send") == 0) {
        char *name = strtok(NULL, " ");
        if (name) {
            if (!send_by_name(name)) {
                printf("? Comando não encontrado: %s\n", name);
            }
        } else {
            printf("? Uso: send <nome>\n");
        }
        
    } else if (strcasecmp(cmd, "protocol") == 0) {
        char *protocol = strtok(NULL, " ");
        if (protocol) {
            if (!send_by_protocol(protocol)) {
                printf("? Protocolo inválido: %s\n", protocol);
            }
        } else {
            printf("? Uso: protocol <NECxx-yy>\n");
        }
        
    } else if (strcasecmp(cmd, "raw") == 0) {
        char *device_str = strtok(NULL, " ");
        char *func_str = strtok(NULL, " ");
        if (device_str && func_str) {
            uint8_t device = (uint8_t)strtol(device_str, NULL, 16);
            uint8_t function = (uint8_t)strtol(func_str, NULL, 16);
            printf("Enviando raw: Device=0x%02X, Function=0x%02X\n", device, function);
            send_ir_command_NEC(device, function);
        } else {
            printf("? Uso: raw <device> <function> (valores em hex)\n");
        }
        
    } else if (strcasecmp(cmd, "help") == 0) {
        show_help();
        
    } else {
        printf("? Comando desconhecido: %s (digite 'help')\n", cmd);
    }
    
    printf("> ");
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    printf("\n\n??????????????????????????????????????????\n");
    printf("?  Transmissor IR - Protocolo NEC       ?\n");
    printf("?  Raspberry Pi Pico                     ?\n");
    printf("??????????????????????????????????????????\n\n");

    pio = pio0;
    
    // Inicializa transmissor NEC
    // nec_tx_init configura DOIS state machines:
    // 1. carrier_burst_sm - gera portadora 38.222kHz no pino IR_TX_PIN
    // 2. carrier_control_sm - controla timing dos pulsos (retorna este SM)
    tx_sm = nec_tx_init(pio, IR_TX_PIN);
    
    // Inicializa receptor NEC (opcional, para debug/feedback)
    rx_sm = nec_rx_init(pio, IR_RX_PIN);

    if (tx_sm == -1 || rx_sm == -1) {
        printf("? ERRO: Não foi possível configurar o PIO\n");
        printf("  Certifique-se que há state machines disponíveis no PIO0\n");
        printf("  TX requer 2 SMs, RX requer 1 SM = total 3 SMs necessários\n");
        return -1;
    }

    printf("? PIO configurado com sucesso!\n");
    printf("  TX: GPIO %d | RX: GPIO %d\n", IR_TX_PIN, IR_RX_PIN);
    printf("  %zu comandos carregados\n\n", NUM_COMMANDS);
    
    show_help();

    // Buffer para entrada de comandos
    static char input_buffer[128];
    static int buf_pos = 0;

    // Loop principal
    while (true) {
        // Lê caractere da entrada
        int c = getchar_timeout_us(1000);
        
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                printf("\n");
                input_buffer[buf_pos] = '\0';
                if (buf_pos > 0) {
                    process_command(input_buffer);
                    buf_pos = 0;
                } else {
                    printf("> ");
                }
            } else if (c == 127 || c == 8) { // Backspace
                if (buf_pos > 0) {
                    buf_pos--;
                    printf("\b \b");
                }
            } else if (buf_pos < sizeof(input_buffer) - 1) {
                input_buffer[buf_pos++] = c;
                printf("%c", c);
            }
        }
        
        // Verifica recepção IR
        if (!pio_sm_is_rx_fifo_empty(pio, rx_sm)) {
            uint32_t rx_frame = pio_sm_get(pio, rx_sm);
            uint8_t rx_device, rx_function;
            
            if (nec_decode_frame(rx_frame, &rx_device, &rx_function)) {
                printf("\n[RX] NEC%02X-%d (Device=0x%02X, Function=0x%02X)\n> ", 
                       rx_device, rx_function, rx_device, rx_function);
            }
        }
    }

    return 0;
}