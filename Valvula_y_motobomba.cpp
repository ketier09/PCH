#include "Valvula_y_motobomba.h"

valvula_motobomba::valvula_motobomba(byte v, byte m)
  : valvula(v), motobomba(m) {}

void valvula_motobomba::set_up(){
  Serial.println(F("\nEnvía '1' para ACTIVAR la valvula y luego la bomba."));
  Serial.println(F("\nEnvía '0' para DESACTIVAR la bomba y luego la valvula."));
  pinMode(valvula, OUTPUT);
  pinMode(motobomba, OUTPUT);
}

void valvula_motobomba::ejecutar_orden(char comando){
      if(comando == '1'){
      digitalWrite(valvula,HIGH);
      digitalWrite(motobomba,HIGH);
      Serial.println(F("\nElectroválvula y motobomba ENCENDIDAS"));
    }else if(comando == '0'){
      digitalWrite(valvula,LOW);
      digitalWrite(motobomba,LOW);
      Serial.println(F("\nElectroválvula y motobomba APAGADAS"));
    }
}

void valvula_motobomba::leer_orden(){
  if (Serial.available() > 0){
    char comando = Serial.read();
    ejecutar_orden(comando);
  }
}
