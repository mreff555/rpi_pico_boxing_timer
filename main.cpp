/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include "lcd.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#include <string>

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

bool is_start_button_pressed(){return false;}

bool is_reset_button_pressed(){return false;}

void ring_bell(){}

void rtc_callback(void){}

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
