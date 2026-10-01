#define F_CPU 8000000UL          // 8 MHz
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define WE_PIN PC4
#define CS_PIN PC5
#define SRAM_SIZE 2048u

static void adc_init(void)
{
    ADMUX  = (1 << REFS0) | (1 << MUX1) | (1 << MUX0);             // AVcc como referencia, canal ADC3
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);            // prescaler /64
    DIDR0  = (1 << ADC3D);                                          // desactiva buffer digital en PC3
}

static uint16_t adc_read(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;                  // el compilador lee ADCL y luego ADCH en el orden correcto
}

static void sram_write(uint16_t addr, uint8_t data)
{
    PORTB = (uint8_t)(addr & 0xFF);
    PORTC = (PORTC & ~0x07) | (uint8_t)((addr >> 8) & 0x07);
    PORTD = data;

    PORTC &= ~(1 << CS_PIN);
    PORTC &= ~(1 << WE_PIN);
    _delay_us(1);
    PORTC |= (1 << WE_PIN);
    PORTC |= (1 << CS_PIN);
}

static void delay_5s(void)
{
    for (uint8_t i = 0; i < 50; i++)
        _delay_ms(100);
}

int main(void)
{
    uint16_t addr = 0;

    DDRB = 0xFF;
    DDRD = 0xFF;
    DDRC = (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << WE_PIN) | (1 << CS_PIN);

    PORTC = (1 << WE_PIN) | (1 << CS_PIN);

    adc_init();

    while (1)
    {
        uint16_t muestra = adc_read();

        sram_write(addr,     (uint8_t)(muestra & 0xFF));
        sram_write(addr + 1, (uint8_t)(muestra >> 8));

        addr += 2;
        if (addr >= SRAM_SIZE) addr = 0;

        delay_5s();
    }
}