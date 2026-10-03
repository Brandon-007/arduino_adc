#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include<avr/io.h>
#include<util/delay.h>
#include<stdint.h>

#define BAUD_RATE 9600
#define UBRR_VALUE ((F_CPU/16UL*BAUD_RATE)-1)

void uart_init(){
    UBRR0H=(unsigned)(UBRR_VALUE>>8);
    UBRR0L=(unsigned)UBRR_VALUE;
    UCSR0B=(1<<RXEN0)|(1<<TXEN0);
    UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
    DDRD|=(1<<DDD1);//set TX (PD1) as output
}

void uart_send(unsigned char data){
    while(!(UCSR0A&(1<<UDRE0)));
    UDR0=data;
}

void uart_print(char *s){
    while(*s) uart_send(*s++);
}

void uart_print_uint(uint16_t n)
{
    char buf[6];
    uint8_t i = 0;

    if (n == 0) {
        uart_send('0');
        return;
    }
    
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    while (i > 0) {
        uart_send(buf[--i]);
    }
}

void adc_init(){
    //1.set reference for voltage to AVcc (with cap at AREF pin)
    ADMUX=(1<<RESF);

    // 2. Enable ADC & set Prescaler to 128
    // 16 MHz / 128 = 125 kHz (ADC requires 50 kHz - 200 kHz clock)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel) {
    // Clear lower 4 bits (MUX3:0) and select requested channel (0-7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Polling: Wait for conversion to complete (ADSC drops LOW)
    while (ADCSRA & (1 << ADSC));

    // Read combined 10-bit result register (ADCL must be read before ADCH)
    return ADC;
}

// Measures exact VCC supply voltage (in mV) using the internal 1.1V Bandgap
uint16_t read_vcc_mv(void) {
    // REFS0 = 1 (AVcc reference)
    // MUX3:0 = 1110 (Selects Internal 1.1V Bandgap Reference)
    ADMUX = (1 << REFS0) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1);

    _delay_ms(2); // Settling time for reference voltage

    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait for completion

    uint16_t adc_val = ADC;

    // Formula: Vcc (mV) = (1.1V * 1024 * 1000) / ADC_value
    uint32_t vcc = 1126400UL / adc_val;
    return (uint16_t)vcc;
}

// -----------------------------------------------------------------------------
// MAIN LOOP
// -----------------------------------------------------------------------------

int main(void) {
    uart_init();
    adc_init();

    while (1) {
        // 1. Measure internal supply voltage (VCC)
        uint16_t vcc_mv = read_vcc_mv();

        // 2. Read analog pin A0 (Channel 0)
        uint16_t a0_raw = adc_read(0);

        // Print over UART
        uart_print("VCC Supply: ");
        uart_print_uint(vcc_mv);
        uart_print(" mV | A0 Raw: ");
        uart_print_uint(a0_raw);
        uart_print("\r\n");

        _delay_ms(1000);
    }
    return 0;
}