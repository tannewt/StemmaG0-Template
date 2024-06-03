#include <stdbool.h>
#include <stddef.h>

#include "build/flash.h"
#include "build/gpiob.h"
#include "build/rcc.h"

// Override __libc_init_array because it crashes.
void __libc_init_array(void) {;}

size_t led_pin = 9;

void I2C1_Handler(void) {
    while (1) {}
}

int main() {
    // Check that the boot pin is active
    if (FLASH->OPTR.nBOOT_SEL) {
        // Wait a little while to improve the chance power is stable.
        for (size_t i = 0; i < 500000; i++) {
            asm("nop");
        }

        // Clear the write lock
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;

        while (FLASH->CR.LOCK) {}

        // Clear the option lock
        FLASH->OPTKEYR = 0x08192A3B;
        FLASH->OPTKEYR = 0x4C5D6E7F;

        while (FLASH->CR.OPTLOCK) {}

        // Clear nBOOT_SEL
        FLASH->OPTR.nBOOT_SEL = false;

        // Wait for any ongoing flash access.
        while (FLASH->SR.BSY) {}

        FLASH->CR.OPTSTRT = true;

        // Wait for the option write to complete.
        while (FLASH->SR.BSY) {}

        // Lock the flash.
        FLASH->CR.LOCK = true;
    }
    RCC->IOPENR.IOPBEN = true;
    // Add your code here.
    GPIOB_REGS->MODER &= ~(0x3 << (led_pin * 2));
    GPIOB_REGS->MODER |= (0x1 << (led_pin * 2));
    uint32_t mask = 1 << led_pin;
    while (true) {
        for (size_t i = 0; i < 500000; i++) {
            asm("nop");
        }
        GPIOB_REGS->BSRR = mask;
        for (size_t i = 0; i < 500000; i++) {
            asm("nop");
        }
        GPIOB_REGS->BSRR = mask << 16;
    }
}