#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define entranceButton PC4        // botão abre/para/fechar motor 1
#define exitButton PC5            // botão abre/para/fechar motor 2
#define entranceMotorPolarity PB5 // troca apolaridade motor 1
#define entranceMotor PB3         // ligar o motor 1
#define exitMotorPolarity PB2     // troca apolaridade motor 2
#define exitMotor PB4             // ligar o motor 2
#define entranceSensor PC0
#define exitSensor PC1
#define entranceEndOfStroke PC2
#define exitEndOfStroke PC3
#define entranceLed PB0
#define exitLed PB1

#define set_bit(Y, bit_x)(Y |= (1 << bit_x))  // ativa o bit x da variável Y (coloca em 1)
#define clr_bit(Y, bit_x)(Y &= ~(1 << bit_x)) // limpa o bit x da variável Y (coloca em 0)
#define tst_bit(Y, bit_x)(Y & (1 << bit_x))   // testa o bit x da variável Y (retorna 0 ou 1)
#define cpl_bit(Y, bit_x)(Y ^= (1 << bit_x))  // inverte bit

enum States { closed, opening, open, closing };

const unsigned char displayHexTable[] ={0x40, 0x79, 0x24, 0x30, 0x19,
                                        0x12, 0x02, 0x78, 0x00, 0x18};
ISR(PCINT1_vect); // Declarar de interrrupc

unsigned int entranceGateState, exitGateState = 0;
unsigned int count = 9 ;

void entranceGate() { //cada vez que esta função e chamada automaticamente 
  // ela verifica o estado anterior é faz as opreções para proximo estado
  if (entranceGateState == States::closed ||
      entranceGateState == States::open) { 
    // abrindo portão
    set_bit(PORTB, entranceMotor);
    clr_bit(PORTB,entranceLed);
     entranceGateState++;
    if (entranceGateState == States::opening && count >0) {
      PORTD = displayHexTable[--count];
    }
   
  } else if (entranceGateState == States::opening ||
             entranceGateState == States::closing) { 
    // parar portão ou fim de curso
    if(entranceGateState == States::opening){
      set_bit(PORTB,entranceLed);
     
    }
    
    clr_bit(PORTB, entranceMotor);
    cpl_bit(PORTB, entranceMotorPolarity);
    entranceGateState == States::opening ? entranceGateState++
                                         : entranceGateState = States::closed;
  }
}

void exitGate() {
  if (exitGateState == States::closed || exitGateState == States::open) {
    set_bit(PORTB, exitMotor);
     exitGateState++;
    clr_bit(PORTB,exitLed);
    if (exitGateState == States::opening && count < 9) {
      PORTD = displayHexTable[++count];
    }
   
  } else if (exitGateState == States::opening ||
             exitGateState == States::closing) {
    clr_bit(PORTB, exitMotor);
     if(exitGateState == States::opening){
      set_bit(PORTB,exitLed);
     
    }
    cpl_bit(PORTB, exitMotorPolarity);
    exitGateState == States::opening ? exitGateState++
                                     : exitGateState = States::closed;
  }
}

void automaticClosing(){
   if( exitGateState == States::open ){
      _delay_ms(500);
      if(exitGateState == States::open){
      	exitGate();
      }
      
    }
    if( entranceGateState == States::open){
      _delay_ms(500);
      if(entranceGateState == States::open){
      	 entranceGate();
      }
       
    }
}

int main() {
  DDRD = 0b11111111; //configurando o PORTD como saida Display 
  DDRB = 0b00111111; // configurando o PORTB como saida
  DDRC = 0x00; // configurando a PORTC como entradas 
  PORTB =(0 << entranceMotor)|(0 << exitMotor); // mudando o estado inicaial das saidas portB;
  PORTD = 0x18; // mudando o estados inicial das saida‹
  PORTC = 0b0111100;// mudando o estados inicial das PORTC

  PCICR = 1 << PCIE1; // Habilitando interrupções por qaulquer mudança de sinal no PORTC
  PCMSK1 = (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | 
    	   (1 << PCINT11) |  (1 << PCINT12) | (1 << PCINT13);
  			// Habilitandos os PINOS para gerar interrupções 

  sei(); // habilitando as interrupções

  while (1) {
 	automaticClosing(); // fechamento automatico dos portões
  }

  return 0;
}


ISR(PCINT1_vect) {
  
   if (!tst_bit(PINC, entranceButton)) { // acionamento do butão portão de entrada
    entranceGate();
   
  }
   else if (!tst_bit(PINC, exitButton)) { // acionamento do butão portão de saida
    exitGate();
   
  }
  
  else if (!tst_bit(PINC, entranceEndOfStroke)) { // FIM DE CURSO
    entranceGate();
   
  }
  else if (!tst_bit(PINC, exitEndOfStroke)) { // FIM DE CURSO
    exitGate();
   
  }
  
 else  if (tst_bit(PINC, entranceSensor)) { 
    // PORTÃO 1 FECHANDO SENSOR PEGOU UM MOVIMENTO
    // PARA O PORTÃO E ABRIR
    if (entranceGateState == States::closing) {
      entranceGate();
      _delay_ms(200);
      entranceGate();
    }
  }

  

 else if (tst_bit(PINC, exitSensor)) {
    // PORTÃO 1 FECHANDO SENSOR PEGOU UM MOVIMENTO
    // PARA O PORTÃO E ABRIR
    if (exitGateState == States::closing) {
      exitGate();
      _delay_ms(200);
      exitGate();
    }
  }

 
}
