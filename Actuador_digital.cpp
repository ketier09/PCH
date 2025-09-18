#include "Actuador_digital.h"

valvula_motobomba::valvula_motobomba(byte v, byte m)
  : pin(p) {}

void valvula_motobomba::set_up(){
  pinMode(pin, OUTPUT);
}

void valvula_motobomba::ejecutar_orden(char comando){
      if(comando == '1'){
      digitalWrite(pin,HIGH);
    }else if(comando == '0'){
      digitalWrite(pin,LOW);
    }
}
