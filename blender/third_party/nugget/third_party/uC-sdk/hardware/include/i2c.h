#pragma once

#include <decl.h>
#include <stdint.h>

#include <gpio.h>

// ought to be generic, cm3-specific for now
typedef enum {
    i2c_0,
    i2c_1,
    i2c_2,
    i2c_3,
} i2c_t;

typedef struct {
    pin_t scl;
    pin_t sda;
    i2c_t i2c;
} i2c_port_t;

ucsdk_static_assert(sizeof(i2c_port_t) <= 6, "i2c_port_t isn't 48 bits-wide");

BEGIN_DECL

void i2c_config(i2c_port_t i2c_port, uint32_t speed);
//void i2c_dma_init(uint8_t id);

//call before initiating a read communication
void i2c_start_read(i2c_t i2c, uint8_t destination);
//call before initiating a write communication
void i2c_start_write(i2c_t i2c, uint8_t destination);
//call after communication ended
void i2c_stop(i2c_t i2c);

void i2c_read(i2c_t i2c, uint8_t *value, uint8_t nb);
void i2c_write(i2c_t i2c, uint8_t *value, uint8_t nb);

//wait for the bus to be ready
void i2c_wait(i2c_t i2c);

//void i2c_read_dma(uint8_t id, uint8_t *buffer, uint8_t nb);
//void i2c_write_dma(uint8_t id, uint8_t *buffer, uint8_t nb);

END_DECL

