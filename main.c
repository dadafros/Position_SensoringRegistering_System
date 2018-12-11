#include "msp430g2553.h"
#define UART_TXD 0x02                                  // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD 0x04                                  // RXD on P1.2 (Timer0_A.CCI1A)                              
#define UART_TBIT_DIV_2 (1000000 / (9600 * 2))         // Conditions for 9600 Baud SW UART, SMCLK = 1MHz
#define UART_TBIT (1000000 / 9600)

const char mensg1[16]="     REGUA      ";      //cria as mensagens transitórias
const char mensg2[16]="   ELETRONICA   ";
const char mensg3[16]="ALUNOS:         ";
const char mensg4[16]="Davi Frossard   ";
const char mensg5[16]="Willian Braga   ";
const char mensg6[16]="Alvaro Resende  ";
const char tela1[16]="Desloc:       cm";       //cria a tela permanente 
const char tela2[16]=" META  CEFET-MG ";

int pulso=0;                                   // declaração das variáveis globais
int desloc=0;
unsigned int escreve;
unsigned char dado;
unsigned char cmd;                                               

unsigned int txData;                           // Globals for full-duplex UART communication        
unsigned char rxBuffer=0;                      // UART internal variable for TX
                                               // Received UART character

void TimerA_UART_tx(unsigned char byte);       // Function prototypes
void TimerA_UART_print(char *string);
//*****************************************************************************
//define a função configura I/O
void conf_io (void)
{
  P1DIR=0xf0;     //Bits mais significativos da Porta 1 configurada como saída
  P2DIR=0x03;     //Bits da porta 2.0 e 2.1 configurados como saída
  P2IE=0x10;      //Habilita interrupções no pino P2.4 
  P2IES=0x10;     //Configura a interrupção no pino P2.4 na borda de descida
  P2IFG=0x00;     //Limpa a flag de interrupção da porta P2, informando que não existem interrupções pendentes	
}
//*****************************************************************************
//define a função delay de com base de tempo de 1ms. O parametro de entrada
//x define o tempo de delay atraves da equação tempo=x*1ms
void delay (unsigned int x)
{
  int y;
  while (x>0)
  {
        for (y=0;y<150;y++);//loop de 1 ms
        x--;
  }
}
//*****************************************************************************
//define a função envia comando para display
void env_com (unsigned char cmd)
{
  P1OUT=(cmd&0xF0);//envia parte alta do comando
  P2OUT=0x00;   //sinais de controle do display E=RS=0
  delay(5);     //tempo de 5 ms
  P2OUT=0x02;   //faz E=1
  delay(5);     //tempo de 5 ms
  P2OUT=0x00;   //faz E=0
  P1OUT=(cmd&0x0F);//envia parte baixa do comando
  P1OUT<<=4;
  P2OUT=0x00;   //sinais de controle do display E=RS=0
  delay(5);     //tempo de 5 ms
  P2OUT=0x02;   //faz E=1
  delay(5);     //tempo de 5 ms
  P2OUT=0x00;   //faz E=0  
}
//*****************************************************************************
//define a função envia dado para display
void env_dado (unsigned char dado)
{
  P1OUT=(dado&0xF0);//envia parte alta do dado
  P2OUT=0x01;   //sinais de controle do display E=0 e RS=1
  delay(5);     //tempo de 5 ms
  P2OUT=0x03;   //faz E=RS=1
  delay(5);     //tempo de 5 ms
  P2OUT=0x01 ;  //faz E=0 e RS=1
  P1OUT=(dado&0x0F);//envia parte baixa do dado
  P1OUT<<=4;
  P2OUT=0x01;   //sinais de controle do display E=0 e RS=1
  delay(5);     //tempo de 5 ms
  P2OUT=0x03;   //faz E=RS=1
  delay(5);     //tempo de 5 ms
  P2OUT=0x01 ;  //faz E=0 e RS=1 
}
//*****************************************************************************
//define a função capaz de escrever mensagens e telas no display
void escreve_dis (void)
{
  env_com(0x80);//seta o endereço inicial da 1ª linha do modulo display
  char i;
  for (i=0;i<16;i++)
  {
    env_dado(mensg1[i]);//escreve mensagem no display
  }
  env_com(0xC0);//seta o endereço inicial da 2ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(mensg2[i]);//escreve mensagem no display
  }
  delay(10000);//espera 10 segundos

  
  env_com(0x80);//seta o endereço inicial da 1ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(mensg3[i]);//escreve mensagem no display
  }
  env_com(0xC0);//seta o endereço inicial da 2ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(mensg4[i]);//escreve mensagem no display
  }
  delay(2000);
      
  env_com(0xC0);//seta o endereço inicial da 2ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(mensg5[i]);//escreve mensagem no display
  }
  delay(2000);
     
  env_com(0xC0);//seta o endereço inicial da 2ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(mensg6[i]);//escreve mensagem no display
  }
  delay(2000);  
   
  
  env_com(0x80);//seta o endereço inicial da 1ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(tela1[i]);//escreve tela permanente no display
  }
  env_com(0xC0);//seta o endereço inicial da 2ª linha do modulo display
  for (i=0;i<16;i++)
  {
    env_dado(tela2[i]);//escreve tela permanente no display
  } 
}
//*****************************************************************************
//define a função programa LCD
void prog_lcd (void)
{
  env_com(0x01);   //Limpa display
  env_com(0x28);   //Fixação das condições de utilização
  env_com(0x0C);   //Controle ativo/inativo do display
  env_com(0x06);   //Fixação do modo de escrita
  escreve_dis();
}
//*****************************************************************************
//define a função de conversão binario para decimal
//converte a variável binária "desloc" para BCD, utiliza os registradores
//R12 (entrada) e R13 (saída).
void bin_dec(void)
{
  char i;
  asm("push R12");
  asm("push R13");
  asm("mov &desloc, r12");
  asm("clr r13");
  for(i=0;i<16;i++)
  {
    asm("rla r12");
    asm("dadd r13,r13");
  }
  asm("mov r13,&desloc");
  asm("pop R13");
  asm("pop R12");
}
//*****************************************************************************
//escreve "desloc" originalmente BCD no display e no PC, desta forma esta função
//converte cada digito BCD destas variáveis no seu correspondente codigo ASCII
void dec_display (void) 
{ 
  escreve=desloc&0xf000;//mascara o digito do milhar
  escreve=desloc>>12;//desloca o digito do milhar p/ a unidade
  escreve+=0x30;//converte para ASCII
  env_dado(escreve);//escreve digito do milhar em ASCII
  TimerA_UART_tx(escreve);//envia digito do milhar ao PC
  escreve=desloc&0x0f00;//mascara o digito da centena
  escreve=desloc>>8;//desloca o digito da centena p/ a unidade
  asm("bic #0xFFF0,escreve");//corrige problema
  escreve+=0x30;//converte para ASCII
  env_dado(escreve);//escreve digito da centena em ASCII
  TimerA_UART_tx(escreve);//envia digito da centena ao PC
  env_dado(0x2E);//escreve vírgula 
  TimerA_UART_tx('.');//envia vírgula ao PC
  escreve=desloc&0x00f0;//mascara o digito da dezena
  escreve=escreve>>4;//desloca o digito da dezena p/ a unidade
  escreve+=0x30;//converte para ASCII
  env_dado(escreve);//escreve digito da dezena em ASCII
  TimerA_UART_tx(escreve);//envia digito da dezena ao PC
  escreve=desloc&0x000f;//mascara o digito da unidade
  escreve+=0x30;//converte para ASCII
  env_dado(escreve);//escreve digito da unidade em ASCII
  TimerA_UART_tx(escreve);//envia digito da unidade ao PC
}
//*****************************************************************************
//envia deslocamento às saídas: Display e PC
void output(void)
{
  if(pulso<0)
   {
      env_com(0x88);//seta a 1ª linha, 8ª casa do modulo display
      env_dado(0x2D);//escreve o sinal de negativo no display
      TimerA_UART_tx('-');//envia sinal de negativo ao PC
      dec_display();
   }
  else
   {
      env_com(0x88);//seta a 1ª linha, 8ª casa do modulo display
      env_dado(0xA0);//escreve espaço vazio no display
      TimerA_UART_tx('+');//envia sinal de positivo ao PC
      dec_display();
   }
} 
//*****************************************************************************
//Rotina da Comunicação Serial
void programaSerial(void){
  DCOCTL = 0x00;                                       // Set DCOCLK to 1MHz
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  P1OUT = 0x00;                                        // Initialize all GPIO

  P1SEL = UART_TXD + UART_RXD;                         // Timer function for TXD/RXD pins
  P1DIR = 0xFF & ~UART_RXD;                            // Set all pins but RXD to output

                                                       // Configures Timer_A for full-duplex UART operation
  TA0CCTL0 = OUT;                                      // Set TXD Idle as Mark = '1'
  TA0CCTL1 = SCS + CM1 + CAP + CCIE;                   // Sync, Neg Edge, Capture, Int
  TA0CTL = TASSEL_2 + MC_2;                            // SMCLK, start in continuous mode
  __enable_interrupt();  
}

unsigned char TimerA_UART_rx()
{
unsigned char dado=rxBuffer;
rxBuffer=0;
return dado;
}

void TimerA_UART_tx(unsigned char byte) {              // Outputs one byte using the Timer_A UART

  while (TACCTL0 & CCIE);                              // Ensure last char got TX'd

  TA0CCR0 = TAR;                                       // Current state of TA counter

  TA0CCR0 += UART_TBIT;                                // One bit time till first bit

  TA0CCTL0 = OUTMOD0 + CCIE;                           // Set TXD on EQU0, Int

  txData = byte;                                       // Load global variable

  txData |= 0x100;                                     // Add mark stop bit to TXData

  txData <<= 1;                                        // Add space start bit
}

void TimerA_UART_print(char *string) {                 // Prints a string using the Timer_A UART

  while (*string)
    TimerA_UART_tx(*string++);
}
//*****************************************************************************
//define a função principal
void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;// Stop watchdog timer to prevent time out reset
  conf_io();
  prog_lcd();
  programaSerial();
  while(1)
  {  
    desloc=abs(pulso);
    bin_dec();
    output();
  }  
}

//*****************************************************************************
//define a rotina de interrução para leitura dos sinais do sensor
#pragma vector=PORT2_VECTOR// Interrupção da porta P2. Quando detectada a borda de descida em P2.4, executa-se o código:
__interrupt void Port_2(void)
{
   if ((0x20 & P2IN))//Sensor deslocando-se para esquerda 
   {
      pulso=pulso-2;
   }
   else//Sensor deslocando-se para direita
   {
      pulso=pulso+2;
   }  
   P2IFG=0x00; // Limpa a flag de interrupção da porta P2, informando que não existem interrupções pendentes
}
//*****************************************************************************
//define a rotina de interrução usada na comunicação serial
#pragma vector = TIMER0_A0_VECTOR                      // Timer_A UART - Transmit Interrupt Handler

   __interrupt void Timer_A0_ISR(void) {

  static unsigned char txBitCnt = 10;

  TA0CCR0 += UART_TBIT;                                // Add Offset to CCRx

  if (txBitCnt == 0) {                                 // All bits TXed?

    TA0CCTL0 &= ~CCIE;                                 // All bits TXed, disable interrupt

    txBitCnt = 10;                                     // Re-load bit counter
  }
  else {
    if (txData & 0x01)
      TA0CCTL0 &= ~OUTMOD2;                            // TX Mark '1'
    else
      TA0CCTL0 |= OUTMOD2;                             // TX Space '0'
  }
  txData >>= 1;                                        // Shift right 1 bit
  txBitCnt--;
}

#pragma vector = TIMER0_A1_VECTOR                      // Timer_A UART - Receive Interrupt Handler

  __interrupt void Timer_A1_ISR(void) {

  static unsigned char rxBitCnt = 8;

  static unsigned char rxData = 0;

  switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {       // Use calculated branching

    case TA0IV_TACCR1:                                 // TACCR1 CCIFG - UART RX

         TA0CCR1 += UART_TBIT;                         // Add Offset to CCRx

         if (TA0CCTL1 & CAP) {                         // Capture mode = start bit edge

           TA0CCTL1 &= ~CAP;                           // Switch capture to compare mode

           TA0CCR1 += UART_TBIT_DIV_2;                 // Point CCRx to middle of D0
         }
         else {
           rxData >>= 1;

           if (TA0CCTL1 & SCCI)                        // Get bit waiting in receive latch
             rxData |= 0x80;
           rxBitCnt--;

           if (rxBitCnt == 0) {                        // All bits RXed?

             rxBuffer = rxData;                        // Store in global variable

             rxBitCnt = 8;                             // Re-load bit counter

             TA0CCTL1 |= CAP;                          // Switch compare to capture mode

             _BIC_SR(LPM0_EXIT);                       // wake up from low power mode.
           }
         }
         break;
        }
}