#include "efm32gg990f1024.h"
#include "em_chip.h"    // required for CHIP_Init() function
  
#define COM_PORT 3 // gpioPortD (USART location #1: PD0 and PD1)
#define ADC_PORT 3 // gpioPortD (ADC Channel 6 location #0: PD6)
#define TX_pin 0
#define ADC_pin 6 // ADC Channel 6
 
void set_decade(uint16_t val);
uint8_t conv_ascii(uint16_t val);
  
uint16_t ms_counter = 0;
char header[] = "\n\rEFM32 Giant Gecko - ADC Example\n\r";
uint8_t digit_array[7] = { 0x20, 0x20, 0x20, 0x20, 0x20, 'm', 'V' }; // Array for displaying ADC result, initialize to "     mV"
  
void TIMER0_IRQHandler(void) {
  TIMER0->IFC = 1; // Clear overflow flag
  ms_counter++;    // Increment counter
}
 
int main() {
  CHIP_Init();                   // This function addresses some chip errata and should be called at the start of every EFM32 application (need em_system.c)
   
  uint8_t i;
  uint16_t adc_result = 0;       // Temp variable for storing ADC conversion results
  uint32_t temp;
   
  // Initialize Clock Tree
  CMU->CTRL |= (1 << 14);        // Set HF clock divider to /2 to keep core frequency <32MHz
  CMU->OSCENCMD |= 0x4;          // Enable XTAL Oscillator
  while(! (CMU->STATUS & 0x8) ); // Wait for XTAL osc to stabilize
  CMU->CMD = 0x2;                // Select HF XTAL osc as system clock source. 48MHz XTAL, but we divided the system clock by 2, therefore our HF clock should be 24MHz
  CMU->HFPERCLKEN0 = (1 << 16) | (1 << 13) | (1 << 5) | (1 << 1); // Enable GPIO, TIMER0, USART1, and ADC0 peripheral clocks
   
  // Initialize GPIO 
  GPIO->P[COM_PORT].MODEL = (1 << 24) | (1 << 4) | (4 << 0); // Configure PD0 as digital output, PD1 and PD6 as input
  GPIO->P[COM_PORT].DOUTSET = (1 << TX_pin);                 // Initialize PD0 high since UART TX idles high (otherwise glitches can occur)
   
  // Setup UART Port for asynch mode, frame format 8-none-1-none
  USART1->CLKDIV = (152 << 6);                               // 152 will give 38400 baud rate (using 16-bit oversampling with 24MHz peripheral clock)
  USART1->CMD = (1 << 11) | (1 << 10) | (1 << 2) | (1 << 0); // Clear RX/TX buffers and shif regs, Enable Transmitter and Receiver
  USART1->IFC = 0x1FF9;                                      // clear all USART interrupt flags
  USART1->ROUTE = 0x103;                                     // Enable TX and RX pins, use location #1 (UART TX and RX located at PD0 and PD1, see EFM32GG990 datasheet for details)
    
  // Setup ADC
  // Timebase bit field = 24, defines ADC warm up period (must be greater than or equal to number of clock cycles in 1us)
  // Prescaler setting = 1: ADC clock = HFPERCLK/2 = 12MHz (ADC clock should be between 32kHz and 13MHz)
  // Oversampling set to 2, no input filter, no need for Conversion Tailgating
  // Warm-up mode = NORMAL (ADC is not kept warmed up between conversions)
  ADC0->CTRL = (24 << 16) | (1 << 8);
   
  // Don't use PRS as input
  // Can use single-cycle acquisition time since we are spacing out our conversions using a timer
  // Use buffered Vdd as reference, use Channel 6 as input to single conversion
  // 12-bit resolution, right-justified, single-ended input, single conversion
  ADC0->SINGLECTRL = (2 << 16) | (6 << 8);
  ADC0->IEN = 0x0; // Disable ADC interrupts
   
  // Setup Timer to trigger conversions
  TIMER0->TOP = 24000;                          // Set TOP value for Timer0
  TIMER0->IEN = 1;                              // Enable Timer0 overflow interrupt
  NVIC_EnableIRQ(TIMER0_IRQn);                  // Enable TIMER0 interrupt vector in NVIC
  TIMER0->CMD = 0x1;                            // Start timer0
   
  // Print Startup Header
  i=0;
  while(header[i] != 0) {
    while( !(USART1->STATUS & (1 << 6)) ); // wait for TX buffer to empty
    USART1->TXDATA = header[i++];            // print each character of the header
  }
   
  while(1) {
    if(ms_counter == 500) {
       
      // Move cursor to start of line
      while( !(USART1->STATUS & (1 << 6)) ); // wait for TX buffer to empty
      USART1->TXDATA = 0x0D;                 // send carriage return
      ADC0->CMD = 0x1;                       // Start Single Conversion
      while(!(ADC0->STATUS & (1 << 16)));    // Wait for single conversion data to become valid
      adc_result = ADC0->SINGLEDATA;         // Store conversion result
       
      // Change hex result to decimal result in mV
      temp = adc_result*3300;
      adc_result = temp/4095;
      set_decade(adc_result); // Divide result into individual characters to be transmitted over UART
       
      // Transmit result characters over UART
      for(i=0; i<7; i++) {
        while( !(USART1->STATUS & (1 << 6)) ); // wait for TX buffer to empty
        USART1->TXDATA = digit_array[i];       // print each character of the test string
      }
       
      ms_counter = 0; // reset counter
    }
  }
}
 
// This function is used to divide a 16-bit value into 5 individual digits
void set_decade(uint16_t val)
{
  uint16_t tmp;
  tmp = val / 10000;
  if (tmp) {
    digit_array[0] = conv_ascii(tmp);
  } else
  digit_array[0] = ' ';
  val = val - tmp*10000;
  tmp = val / 1000;
  if (tmp) {
    digit_array[1] = conv_ascii(tmp);
  } else
  digit_array[1] = ' ';
  val = val - tmp*1000;
  tmp = val / 100;
  if (tmp || digit_array[1]!=' ') {
    digit_array[2] = conv_ascii(tmp);
  } else digit_array[2] = ' ';
  val = val - tmp*100;
  tmp = val / 10;
  if (tmp  || digit_array[2]!=' ' || digit_array[1]!=' ') {
    digit_array[3] = conv_ascii(tmp);
  } else digit_array[3] = ' ';
  val = val - tmp*10;
  digit_array[4] = conv_ascii(val);
}
 
// This function is used to covert the character to its ASCII value.
uint8_t conv_ascii(uint16_t val)
{
  if (val<= 0x09) {
    val = val + 0x30;
  }
  if ((val >= 0x0A) && (val<= 0x0F)) {
    val = val + 0x37;
  }
  return val;
}