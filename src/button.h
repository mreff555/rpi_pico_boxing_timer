/**
 * @file button.hpp
 * @author Dan Feerst (feerstd@gmail.com)
 * @brief
 * @version 0.1
 * @date 2026-03-13
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef BUTTON_H
#define BUTTON_H

#include "common.h"
#include <cstdint>
#include <ctime>
#include <string>

#include "pico/stdlib.h"
#include "hardware/pio.h"

enum ButtonState
{
  DEFAULT_STATE,
  RISING_EDGE,
  LONG_HOLD,
  FALLING_EDGE
};

class Button
{
public:
  /**
   * @brief Creates a button object which will monitor
   * the specified GPIO.  There is no error checking for
   * valid GPIO's at the moment.  Partially because I am
   * lazy and partially because it will very from model
   * to model.
   *
   * @param pio The PIO instance to use
   * @param gpio GPIO pin number
   * @param buttonDesc Optional description
   */
  Button(PIO pio, uint8_t gpio, std::string buttonDesc = "");

  /**
   * @brief Destroy the Button object
   *
   */
  virtual ~Button() = default;

  /**
   * @brief Update the button state.  Possible values are
   * -  DEFAULT_STATE
   * -  RISING_EDGE
   * -  LONG_HOLD
   * -  FALLING_EDGE
   *
   * @param newValue
   */
  void update();

  /**
   * @brief Return the buttons GPIO value
   *
   * @return uint8_t GPIO
   */
  uint8_t getGpio() const;

  /**
   * @brief Get the State object
   *
   * @return ButtonState
   */
  ButtonState getState() const;

  /**
   * @brief Get the Time Stamp object
   *
   * @return time_t
   */
  time_t getTimeStamp() const;

  /**
   * @brief Get button description
   *
   * @return std::string
   */
  std::string getButtonDesc() const;

  /**
   * @brief If gpio's match, update timestamp and state and
   * return true.  Otherwise return false
   *
   * @param rhs Button
   * @return true GPIO's match.  Timestamp and stat updated.
   * @return false No match.  Nothing updated.
   */
  bool operator<<(const Button &rhs);

private:

bool timeElapsedExceeds(time_t startTime, uint32_t seconds);

  // gpio number which button is acting on
  uint8_t gpioId;

  // current button state
  ButtonState buttonState;

  // last button update
  time_t timeStamp;

  // button description
  std::string mButtonDesc;
  
  PIO pio;
  
  uint sm;
  
  uint offset;
  
};

#endif /* BUTTON_H */
