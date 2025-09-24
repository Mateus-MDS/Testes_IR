/**
 * custom_ir.h - Header do Protocolo IR Customizado para Raspberry Pi Pico
 * 
 * Copyright (c) 2024
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUSTOM_IR_H
#define CUSTOM_IR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tipos de sinais IR disponíveis
typedef enum {
    IR_OFF,      // Desligar ar condicionado
    IR_ON,       // Ligar ar condicionado
    IR_TEMP_22,  // Temperatura 22°C
    IR_TEMP_20,  // Temperatura 20°C
    IR_FAN_1,    // Ventilador nível 1
    IR_FAN_2     // Ventilador nível 2
} ir_signal_type_t;

// Estrutura para sinais RAW
typedef struct {
    const uint16_t* data;
    size_t length;
} ir_raw_signal_t;

/**
 * Inicializa o sistema IR no pino especificado
 * 
 * @param gpio_pin Pino GPIO para saída IR (recomendado: 2)
 * @return true se inicializado com sucesso, false caso contrário
 */
bool custom_ir_init(uint gpio_pin);

/**
 * Envia um sinal RAW diretamente
 * 
 * @param signal Array com os tempos em microssegundos
 * @param length Quantidade de elementos no array
 */
void send_raw_signal(const uint16_t* signal, size_t length);

/**
 * Envia um comando específico pré-definido
 * 
 * @param command Tipo do comando a ser enviado
 * @return true se enviado com sucesso, false caso contrário
 */
bool send_ir_command(ir_signal_type_t command);

// Funções de conveniência para comandos específicos

/**
 * Desliga o ar condicionado
 */
void turn_off_ac(void);

/**
 * Liga o ar condicionado
 */
void turn_on_ac(void);

/**
 * Define temperatura para 22°C
 */
void set_temp_22c(void);

/**
 * Define temperatura para 20°C
 */
void set_temp_20c(void);

/**
 * Define ventilador para nível 1
 */
void set_fan_level_1(void);

/**
 * Define ventilador para nível 2
 */
void set_fan_level_2(void);

/**
 * Função de demonstração do sistema IR
 */
void ir_demo(void);

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_IR_H