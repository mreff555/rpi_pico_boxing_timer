/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#include <string>

/* Example code to drive a 16x2 LCD panel via a I2C bridge chip (e.g. PCF8574)

   NOTE: The panel must be capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefor I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO 4 (pin 6)-> SDA on LCD bridge board
   GPIO 5 (pin 7)-> SCL on LCD bridge board
   3.3v (pin 36) -> VCC on LCD bridge board
   GND (pin 38)  -> GND on LCD bridge board
*/
// commands
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_CURSORSHIFT = 0x10;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETCGRAMADDR = 0x40;
const int LCD_SETDDRAMADDR = 0x80;

// flags for display entry mode
const int LCD_ENTRYSHIFTINCREMENT = 0x01;
const int LCD_ENTRYLEFT = 0x02;

// flags for display and cursor control
const int LCD_BLINKON = 0x01;
const int LCD_CURSORON = 0x02;
const int LCD_DISPLAYON = 0x04;

// flags for display and cursor shift
const int LCD_MOVERIGHT = 0x04;
const int LCD_DISPLAYMOVE = 0x08;

// flags for function set
const int LCD_5x10DOTS = 0x04;
const int LCD_2LINE = 0x08;
const int LCD_8BITMODE = 0x10;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;

const int LCD_ENABLE_BIT = 0x04;

// By default these LCD display drivers are on bus address 0x27
static int addr = 0x27;

// Modes for lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

#define MAX_LINES      2
#define MAX_CHARS      16

/* Quick helper function for single byte transfers */
void i2c_write_byte(uint8_t val) {
#ifdef i2c_default
    i2c_write_blocking(i2c_default, addr, &val, 1, false);
#endif
}

void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

// go to location on LCD
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

static void inline lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_string(const char *s) {
    while (*s) {
        lcd_char(*s++);
    }
}

void lcd_init() {
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}

// I don't care about the date.  This is just a starting point for the clock.
datetime_t t = {
    .year  = 2024,
    .month = 6,
    .day   = 1,
    .dotw  = 6, // Day of the week, 0 is Sunday, so 6 is Saturday
    .hour  = 12,
    .min   = 0,
    .sec   = 0
};

int round = 1;

enum class State
{
    STOPPED,
    ACTIVE,
    COOLDOWN,
    ACTIVE_PAUSED,
    COOLDOWN_PAUSED
};

State current_state = State::STOPPED;

std::string seconds_to_time_string(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    char buffer[9]; // HH:MM:SS
    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
    return std::string(buffer);
}

// These functions are specific to a 16x2 LCD display and imaginary components.
void write_current_time_state()
{
    datetime_t t;
    rtc_get_datetime(&t);
    int total_seconds = t.hour * 3600 + t.min * 60 + t.sec;
    std::string time_str = seconds_to_time_string(total_seconds);
    
    lcd_set_cursor(0, 0);
    lcd_string(time_str.c_str());
}

void write_current_round_state(){}

bool is_start_button_pressed(){}

bool is_reset_button_pressed(){}

void ring_bell(){}

void rtc_callback(void)
{
    ring_bell();
}

void set_alarm_for_round()
{
    datetime_t t;
    rtc_get_datetime(&t);
    t.min += 3;
    rtc_set_alarm(&t, &rtc_callback);
    rtc_enable_alarm();
}

void set_alarm_for_cooldown()
{
    datetime_t t;
    rtc_get_datetime(&t);
    t.min += 1;
    rtc_set_alarm(&t, &rtc_callback);
    rtc_enable_alarm();
}




int main() {
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lcd_1602_i2c example requires a board with I2C pins
#else
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    lcd_init();
    rtc_init();
    rtc_set_datetime(&t);
    sleep_us(64);

    lcd_clear();
    current_state = State::STOPPED;
    if(rtc_running())
    {
        while(true)
        {
            if(current_state == State::STOPPED)
            {
                // If start button is pressed, set to active state
                // if reset button is pressed, restart the boxing clock
                lcd_clear();
                lcd_set_cursor(0, 0);
                lcd_string("Clock Stopped");
                lcd_set_cursor(1, 0);
                lcd_string("Press Start");
                if(is_start_button_pressed())
                {
                    current_state = State::ACTIVE;
                }
                else if(is_reset_button_pressed())
                {
                    round = 1;
                    current_state = State::COOLDOWN;
                    set_alarm_for_cooldown();
                }
            }
            else if(current_state == State::ACTIVE)
            {
                // If start button is pressed, set to active paused state
                // If reset button is pressed, increment the round

                lcd_clear();
                

                // if(is_start_button_pressed())
                // {
                //     current_state = State::ACTIVE_PAUSED;
                // }
            }
            else if(current_state == State::COOLDOWN)
            {
                // If start button is pressed, set to cooldown paused state
                // If reset button is pressed, increment the round
                
                // write_current_round_state();
            }
            else if(current_state == State::ACTIVE_PAUSED)
            {
                // If start button is pressed, set to active state
                // If reset button is pressed, increment the round


                // write_current_time_state();
                // lcd_set_cursor(1, 0);
                // lcd_string("Paused");
                // if(is_start_button_pressed())
                // {
                //     current_state = State::ACTIVE;
                // }
            }
            else if(current_state == State::COOLDOWN_PAUSED)
            {
                // If start button is pressed, set to cooldown state
                // If reset button is pressed, increment the round


                // write_current_round_state();
                // lcd_set_cursor(1, 0);
                // lcd_string("Paused");
                // if(is_start_button_pressed())
                // {
                //     current_state = State::COOLDOWN;
                // }
            }
            else{}

            sleep_us(1000); // Base clock 1000Hz
        }

    }
#endif
}
