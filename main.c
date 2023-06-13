#include <stdbool.h>
#include <stddef.h>

#include "lib/cmsis_device_g0/Include/stm32g030xx.h"

// Override __libc_init_array because it crashes.
void __libc_init_array(void) {;}

size_t led_pin = 9;

void main() {
    // Check that the boot pin is active
    if ((FLASH->OPTR & FLASH_OPTR_nBOOT_SEL) != 0) {

        // Clear the write lock
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;

        // Clear the option lock
        FLASH->OPTKEYR = 0x08192A3B;
        FLASH->OPTKEYR = 0x4C5D6E7F;

        // Clear nBOOT_SEL
        FLASH->OPTR &= ~FLASH_OPTR_nBOOT_SEL;

        // Wait for any ongoing flash access.
        while ((FLASH->SR & FLASH_SR_BSY1) != 0) {}

        FLASH->CR |= FLASH_CR_OPTSTRT;

        // Wait for the option write to complete.
        while ((FLASH->SR & FLASH_SR_BSY1) != 0) {}

        // Lock the flash.
        FLASH->CR |= FLASH_CR_LOCK;
    }
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    // Add your code here.
    GPIOB->MODER &= ~(0x3 << (led_pin * 2));
    GPIOB->MODER |= (0x1 << (led_pin * 2));
    uint32_t mask = 1 << led_pin;
    while (true) {
        for (size_t i = 0; i < 500000; i++) {
            asm("nop");
        }
        GPIOB->BSRR = mask;
        for (size_t i = 0; i < 500000; i++) {
            asm("nop");
        }
        GPIOB->BSRR = mask << 16;
    }
}