#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <cstdio>

#define BUTTON_PIN 26
#define LED_PIN 7
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define LCD_ADDRESS 0x27 // Change this to your LCD's I2C address

volatile int counter = 0;
volatile bool counting = false;

void button_callback(uint gpio, uint32_t events) {
    counting = !counting;
}

void lcd_send_byte(uint8_t bits, uint8_t mode) {
    uint8_t high = mode | (bits & 0xF0) | 0x08;
    uint8_t low = mode | ((bits << 4) & 0xF0) | 0x08;

    uint8_t buf[] = { (uint8_t)(high | 0x04), high, (uint8_t)(low | 0x04), low };
    i2c_write_blocking(I2C_PORT, LCD_ADDRESS, buf, 4, false);
}

void lcd_clear() {
    lcd_send_byte(0x01, 0);
    sleep_ms(2);
}

void lcd_set_cursor(uint8_t line, uint8_t position) {
    uint8_t addr = line == 1 ? 0x80 : 0xC0;
    addr += position;
    lcd_send_byte(addr, 0);
}

void lcd_print(const char *s) {
    while (*s) {
        lcd_send_byte(*s++, 1);
    }
}

int main() {
    stdio_init_all();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize LCD
    lcd_send_byte(0x33, 0); // Initialize
    lcd_send_byte(0x32, 0); // Set to 4-bit mode
    lcd_send_byte(0x06, 0); // Cursor move direction
    lcd_send_byte(0x0C, 0); // Turn cursor off
    lcd_clear(); // Clear screen

    char buffer[32];

    while (true) {
        if (counting) {
            counter++;
            // Update LCD
            lcd_set_cursor(0, 0);
            sprintf(buffer, "Counter: %d", counter);
            lcd_print(buffer);

            gpio_put(LED_PIN, 1);
            sleep_ms(500);
            gpio_put(LED_PIN, 0);
            sleep_ms(500);
        }
    }

    return 0;
}
