#include <stdbool.h>
#include <stddef.h>

#include "build/flash.h"
#include "build/gpiob.h"
#include "build/i2c.h"
#include "build/rcc.h"
#include "build/nvic.h"

// Override __libc_init_array because it crashes.
void __libc_init_array(void) {;}

size_t led_pin = 9;

size_t byte_count = 0;
uint8_t command = 0;
bool read_in_progress = false;
bool write_in_progress = false;
uint32_t data = 0;
uint8_t next_transmit = 0;

volatile size_t high_delay = 500000;
volatile size_t low_delay = 500000;

size_t read_stemma_command(uint8_t command, size_t byte_count) {
    if (command == 1) {
        return high_delay;
    } else if (command == 3) {
        return low_delay;
    }
    return 0xadaffada;
}

void write_stemma_command(uint8_t command, size_t byte_count, size_t data) {
    if (command == 2) {
        high_delay = data;
    } else if (command == 4) {
        low_delay = data;
    }
}

void I2C1_Handler(void) {
    I2C_Type* i2c = I2C1;
    if (i2c->ISR.RXNE) {
        if (command == 0) {
            command = i2c->RXDR.RXDATA;
            // We don't know if we'll read or write, so queue up as if the controller is reading.
            data = read_stemma_command(command, 0);
        } else {
            data |= i2c->RXDR.RXDATA << ((byte_count - 1) * 8);
            if (byte_count % 4 == 3) {
                write_stemma_command(command, byte_count, data);
                data = 0;
            }
            byte_count++;
        }
    }
    if (i2c->ISR.TXE) {
        i2c->TXDR.TXDATA = next_transmit;
        next_transmit = data >> (byte_count * 8);
        if (byte_count % 4 == 3) {
            data = read_stemma_command(command, byte_count);
        }
        byte_count++;
    }
    if (i2c->ISR.STOPF) {
        // TODO: Complete a write.
        write_stemma_command(command, byte_count, data);
        i2c->ISR.STOPF = true; // Clear the interrupt.
        command = 0;
        next_transmit = 0;
    }
}

void init_stemma(void) {
    RCC->CCIPR.I2C1SEL = 2; // HSI16
    RCC->APBENR1.I2C1EN = true;

    I2C_Type* i2c = I2C1;
    i2c->CR1.PE = false;

    // I2CCLK = 16mhz and I2C is 100khz
    I2C_TIMINGR_Type timingr = {.PRESC = 3,
                .SCLL = 0x13,
                .SCLH = 0xF,
                .SDADEL = 0x2,
                .SCLDEL = 0x4};
    i2c->TIMINGR = timingr;
    I2C1_REGS->OAR1 = 0x30 << 1;
    // i2c->OAR1.OA1_7_1 = 0x10 >> 2;
    // i2c->OAR1.OA1MODE = false;
    I2C1_REGS->OAR1 |= 1 << 15;
    // i2c->OAR1.OA1EN = true;
    i2c->CR1.NOSTRETCH = true;
    i2c->CR1.RXIE = true;
    i2c->CR1.TXIE = true;
    i2c->CR1.PE = true;

    NVIC->ISER = 1 << 23;
}

int main() {
    // Check that the boot pin is active
    // if (FLASH->OPTR.nBOOT_SEL) {
    //     // Wait a little while to improve the chance power is stable.
    //     for (size_t i = 0; i < 500000; i++) {
    //         asm("nop");
    //     }

    //     // Clear the write lock
    //     FLASH->KEYR = 0x45670123;
    //     FLASH->KEYR = 0xCDEF89AB;

    //     while (FLASH->CR.LOCK) {}

    //     // Clear the option lock
    //     FLASH->OPTKEYR = 0x08192A3B;
    //     FLASH->OPTKEYR = 0x4C5D6E7F;

    //     while (FLASH->CR.OPTLOCK) {}

    //     // Clear nBOOT_SEL
    //     FLASH->OPTR.nBOOT_SEL = false;

    //     // Wait for any ongoing flash access.
    //     while (FLASH->SR.BSY) {}

    //     FLASH->CR.OPTSTRT = true;

    //     // Wait for the option write to complete.
    //     while (FLASH->SR.BSY) {}

    //     // Lock the flash.
    //     FLASH->CR.LOCK = true;
    // }
    init_stemma();
    RCC->IOPENR.IOPBEN = true;

    GPIOB->AFRL.AFSEL6 = 6; // Set to AF6, I2C
    GPIOB->AFRL.AFSEL7 = 6;
    GPIOB_REGS->OTYPER = 1 << 6 | 1 << 7; // Switch to open drain
    GPIOB->MODER.MODER6 = 2; // Enable AF mode
    GPIOB->MODER.MODER7 = 2;

    // Add your code here.
    GPIOB_REGS->MODER &= ~(0x3 << (led_pin * 2));
    GPIOB_REGS->MODER |= (0x1 << (led_pin * 2));
    uint32_t mask = 1 << led_pin;
    while (true) {
        for (size_t i = 0; i < high_delay; i++) {
            asm("nop");
        }
        GPIOB_REGS->BSRR = mask;
        for (size_t i = 0; i < low_delay; i++) {
            asm("nop");
        }
        GPIOB_REGS->BSRR = mask << 16;
    }
}