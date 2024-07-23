#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

#include "hardware.h"

#include "gpio.h"

GPIO_TypeDef * const stm32f10x_gpio_ports[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG };

void gpio_config(pin_t pin, pin_dir_t dir, pull_t pull) {
    //Clock the port
    RCC_APB2PeriphClockCmd(1 << (pin.port + 2), ENABLE);

    GPIO_InitTypeDef def;
    def.GPIO_Pin = 1 << pin.pin;
    def.GPIO_Speed = GPIO_Speed_50MHz;
    if (dir) //output
    {
      /* GPIO_Mode_Out_OD */
      def.GPIO_Mode = GPIO_Mode_Out_PP;
    }
    else //input
    {
      switch(pull)
      {
        case pull_none:
          def.GPIO_Mode = GPIO_Mode_IN_FLOATING;
          break;
        case pull_up:
          def.GPIO_Mode = GPIO_Mode_IPU;
          break;
        case pull_down:
          def.GPIO_Mode = GPIO_Mode_IPD;
          break;
      }
    }
    GPIO_Init(stm32f10x_gpio_ports[pin.port], &def);
}

void gpio_config_alternate(pin_t pin, pin_dir_t dir, pull_t pull, uint8_t af) {
    if (af > 15)
        return;
    //Clock the port
    RCC_APB2PeriphClockCmd(1 << pin.port, ENABLE);

    GPIO_InitTypeDef def;
    def.GPIO_Pin = 1 << pin.pin;
    def.GPIO_Speed = GPIO_Speed_50MHz;
    if (dir)
        def.GPIO_Mode = GPIO_Mode_AF_PP; //output : Push Pull
    else
        def.GPIO_Mode = GPIO_Mode_AF_OD; //input : Open Drain

    GPIO_Init(stm32f10x_gpio_ports[pin.port], &def);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void gpio_set(pin_t pin, int enabled) {
    if (enabled)
        GPIO_SetBits(stm32f10x_gpio_ports[pin.port], 1 << pin.pin);
    else
        GPIO_ResetBits(stm32f10x_gpio_ports[pin.port], 1 << pin.pin);
}

uint8_t gpio_get(pin_t pin) {
    return GPIO_ReadInputDataBit(stm32f10x_gpio_ports[pin.port], 1 << pin.pin);
}
