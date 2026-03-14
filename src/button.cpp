#include "common.h"
#include "button.h"
#include "button.pio.h"

Button::Button(PIO pio, uint8_t gpio, std::string buttonDesc)
    : pio(pio), gpioId(gpio), buttonState(DEFAULT_STATE), sm(0), offset(0)
{
    rtc_get_datetime(&timeStamp);

    if(buttonDesc == "")
    {
        this->mButtonDesc = std::string("gpio ") + std::to_string(gpio);
    }
    else
    {
        this->mButtonDesc = buttonDesc;
    }

    // Claim an unused state machine
    sm = pio_claim_unused_sm(this->pio, true);
    
    // Add the program
    offset = pio_add_program(this->pio, &button_program);
    
    // Initialize the program
    button_program_init(this->pio, sm, offset, gpio);
    printf("Using gpio %d\n", gpio);  // DEBUG
}

void Button::update()
{
    switch(buttonState)
    {
        case DEFAULT_STATE:
            if(pio_sm_get(pio, sm) == 1)
            {
                buttonState = RISING_EDGE;
            }
            break;
        case RISING_EDGE:
            if(pio_sm_get(pio, sm) == 1)
            {
                if(timeElapsedExceeds(timeStamp, BUTTON_LONG_HOLD_SECONDS))
                {
                    buttonState = LONG_HOLD;
                }
            }
            else if(pio_sm_get(pio, sm) == 0)
            {
                buttonState = FALLING_EDGE;
            }
            break;
        case LONG_HOLD:
            if(pio_sm_get(pio, sm) == 0)
            {
                buttonState = FALLING_EDGE;
            }
            break;
        case FALLING_EDGE:
            if(pio_sm_get(pio, sm) == 0)
            {
                if(timeElapsedExceeds(timeStamp, BUTTON_DEFAULT_STATE_TRANSITION_SECONDS))
                {
                    buttonState = DEFAULT_STATE;
                }
            }
            else if(pio_sm_get(pio, sm) == 1)
            {
                buttonState = RISING_EDGE;
            }
            break;
    } 
    rtc_get_datetime(&timeStamp);
}

uint8_t Button::getGpio() const { return gpioId; }

ButtonState Button::getState() const { return buttonState; }

datetime_t Button::getTimeStamp() const { return timeStamp; }

std::string Button::getButtonDesc() const { return mButtonDesc; }

bool Button::operator<<(const Button &rhs)
{
    bool success = false;
    if (this->gpioId == rhs.getGpio())
    {
        this->buttonState = rhs.getState();
        this->timeStamp = rhs.getTimeStamp();
        success = true;
    }
    return success;
}

bool Button::timeElapsedExceeds(datetime_t startTime, uint32_t seconds)
{
    datetime_t currentTime;
    rtc_get_datetime(&currentTime);
    
    uint32_t startSeconds = startTime.hour * 3600 + startTime.min * 60 + startTime.sec;
    uint32_t currentSeconds = currentTime.hour * 3600 + currentTime.min * 60 + currentTime.sec;
    
    return (currentSeconds - startSeconds) >= seconds;
}