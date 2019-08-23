//Kod za periferijski uredjaj
#include <SoftwareSerial.h>  //uvoz bib. za serijsku (UART) vezu izmedju glavnog i perif. uredjaja
//#include <Wire.h> //uvoz biblioteke za I2C protokol
#define ATM_GRS 254
byte podatak; //definisemo 8-mobitni podatak koji saljemo preko UART-a ka glavnom uredjaju
byte zv = 5; //na nozici 5 definisemo izlazni uredjaj - zvucnik
byte sed1 = 2;
byte sed2 = 3;
byte sed3 = 4;
SoftwareSerial soft_ser(10,11);// pozivanjem konstruktora klase pravimo objekat soft_ser kome
//prosledjujemo dva parametra
//10 - prijemni pin(Rx)
//11 - predajni pin (Tx)

void setup() {
  //definisanje ulazno-izlaznih pinova
  pinMode(sed1,OUTPUT);
  pinMode(sed2,OUTPUT);
  pinMode(sed3,OUTPUT);
  pinMode(zv,OUTPUT);
  
  soft_ser.begin(9600); //zapocni softversku serijsku vezu (UART) ka glavnom uredjaju

}

//posalji zvucni signal
//note D i G
void ton_1(){
  tone(zv, 294, 3830);
  delay(200);
  tone(zv, 392, 2550);
  delay(200);
  noTone(zv);
  delay(1000);
}
//posalji zvucni signal 2
void ton_2(){
  tone(zv, 200, 3830);
  delay(150);
  noTone(zv);
  delay(50);
  tone(zv, 200, 2550);
  delay(150);
  noTone(zv);
  delay(1000);
}
//funkcije za upravljanje SED
void usve(){
  digitalWrite(sed1, HIGH);
  digitalWrite(sed2, HIGH);
  digitalWrite(sed3, HIGH); 
  ton_1();
}
void isve(){
  digitalWrite(sed1, LOW);
  digitalWrite(sed2, LOW);
  digitalWrite(sed3, LOW);
  ton_2(); 
}
//funkcija za citanje podatka sa serijske veze i slanje odgovora
void proveri_odgovori(byte ppodatak){
	if (ppodatak == 1){
    usve();
    soft_ser.write(3);
    return;
  }
  else if (ppodatak == 2){
    isve();
    soft_ser.write(4);
    return;
  }
  soft_ser.write(ATM_GRS);//ukoliko primljena naredba ne postoji vrati kod greske
}
void loop() {
  
  //ukoliko je serijska magistrala slobodna (!255) procitaj dolzani paket i posalji odgovor
  if (soft_ser.available()>0){
  proveri_odgovori(soft_ser.read());
  
} 

}
