//##########################################################################################################################
//Biblioteke
#include <ESP8266WiFi.h> //uvoz bib. za WiFi fje ESP8266-ice
#include <ESP8266WebServer.h> //uvoz bib. za mrezni server
#include <LiquidCrystal_I2C.h>// uvoz bib. za upravljanje LCD-om preko I2C protokola
#include <Wire.h> //uvoz bib. I2C protokola
#include <SoftwareSerial.h> //uvoz. bib. za softversku serijsku komunikaciju (jer je serijski port zauzet)
//definisemo konstante za UART poruke
#define USVE_ATM 3 
#define ISVE_ATM 4
#define greska_ATM 254
//#########################################################################################################################
//Promenljive
//Mrezni server i WiFi
const char* ssid ="MREZA"; //"naziv_WLAN_mreze"; // char* pokazivac na ssid (ssid - naziv wifi mreze)
const char* sifra ="100pipirotskicilim";// "sifra_WLAN_mreze"; //sifra za povezivanje na WLAN
byte port_http = 80; //definisemo port za mrezni server
byte podatak; //8-mobitan podatak za prenos preko UART serijske veze

//Promenljive u kojima cuvamo stanja SED
String plava_LED_stanje = "ИС";
String zelena_LED_stanje = "ИС";
String crvena_LED_stanje = "ИС";
String zuta_LED_stanje = "ИС";
byte LED_stanje_atmega_sveu = 0; // "ИС";
byte LED_stanje_atmega_svei = 1; // "УК";
static String p_stanje = "LOW";
static String ze_stanje = "LOW";
static String c_stanje = "LOW";
static String zu_stanje = "LOW";

//Objekti
ESP8266WebServer server(port_http); 
//pravimo objekat server pozivanjem konstruktora klase kome prosledjujemo parametar port_http 
LiquidCrystal_I2C ekran(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//pravimo objekat ekran pozivanjem konstruktora klase kome prisledjumemo 10 parametara
//0x27 - adresa I2C uredjaja
// 2,1,0,4,5,6,7 - definisemo raspored nozica na ploci LCD-a
//3 - pin LED-a za osvetljenje displeja
// POSITIVE/NEGATIVE (uklj. isklj. osvetljenje LCD-a)

SoftwareSerial soft_ser(2, 14); //pravimo objekat soft_ser pozivanjem konstruktora klase
//kome prosledjujemo dva parametra:
//2 - Prijemni pin (Rx)
//14 - Predajni pin (Tx)

//##########################################################################################################################
//Pocetna podesavanja kontrolera i funkcije

//WLAN
void povezi_WLAN() {
  //Zapocinjanje pridruzivanja WLAN mrezi
  WiFi.begin(ssid, sifra); //Povezi se na WLAN sa parametrima ssid,sifra
  Serial.print("Повезивање на бежичну мрежу ");//Proces povezivanja pratimo na Serijskom monitoru
  Serial.print(ssid);
  Serial.println("...");
  ekran_pisi("POVEZIVANJE", "U TOKU... "); //Takodje, korisnik moze da nadgleda proces povezivanja na WLAN preko LCD-a

  //Petlja u kojoj se uredjaj povezuje na bezicnu mrezu
  int br = 0;
  //Sve dok status uredjaja nije WL_CONNECTED cekaj 1s kako bi se izvrsilo povezivanje
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++br);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.print("Уређај успешно повезан на мрежу ");
  Serial.print(ssid);
  Serial.println('.');
  Serial.print("Додељена IP адреса уређаја:     ");
  Serial.println(WiFi.localIP());//Ispisi adresu uredjaja dodeljenu od strane DHCP-a
  ekran_pisi("Povezan na WLAN:", ssid);
  //ekran_pisi(ssid,(String)WiFi.localIP());
}
//Serverske funkcije
void pokreni_SERVER() {
  //Definisanje parametara servera
  //Podesavamo parametre servera, tj. kako odgovara na zahteve klijenata
  //GET - klijent ,,uzima`` sa servera, POST - klijent ,,salje`` na server
  //Definisemo na koji nacin ce server da obradi klijentski zahtev (GET ili POST) i koju funkciju poziva za obradu zahteva
  server.on("/", HTTP_GET, serv_odg_uspesan);
  server.on("/plava", HTTP_POST, serv_odg_plava); //npr. pozovi fju serv_odg_plava kada je napravljen POST zahtev za URI /plava
  server.on("/zelena", HTTP_POST, serv_odg_zelena);
  server.on("/crvena", HTTP_POST, serv_odg_crvena);
  server.on("/zuta", HTTP_POST, serv_odg_zuta);
  server.on("/usve", HTTP_POST, serv_odg_usve);
  server.on("/isve", HTTP_POST, serv_odg_isve);
  server.on("/usve_atmega", HTTP_POST, serv_odg_usve_atmega);
  server.on("/isve_atmega", HTTP_POST, serv_odg_isve_atmega);
  server.onNotFound(serv_odg_nijeNadjeno); //fja koja se poziva u slucaju da zahtev nije odgovarajuci

  //Pokretanje servera
  server.begin(port_http);//Pokreni server na portu 80
  //Pratimo proces pokretanja servera
  Serial.print("HTTP сервер покренут на порту ");
  Serial.print(port_http);
  Serial.println(".");
}
//Funkcija koja vraca korisniku html stranicu - pocetnu stranu
void serv_odg_uspesan()
{
  server.send(200, "text\html",
              "<html> "
              "<head>"
              "<meta http-equiv=\"refresh\" content=\"3\" charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width\" >"
              "<style>"
              "form"
              "{"
              "display: block;"
              "display: table;"
              "margin: 0 auto;"
              "margin-top:0em;"
              "margin-bottom: 0em;"
              "}"
              "table, th, td {"
              " border: 1px solid black;"
              "}"
              "</style></head>"
              "<body style=\"background-color:powderblue;\">"
              "<form><table>"
              "<tr>"
              "<th colspan=\"2\">Управљање СЕД</th>"
              "</tr>"
              "<tr>"
              " <th colspan=\"2\">ESP8266</th>"
              "</tr>"
              "<tr>"
              "<td><form></form></td>"
              "<td></td>"
              "</tr>"
              "<tr>"
              "<td><form action=\"/plava\" method=\"POST\"><input type=\"submit\" value=\"      Плава СЕД     \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td><b><font color=#ff471a>" + plava_LED_stanje + "</font><b/></td>"
              "</tr>"
              "<tr>"
              "<td><form action=\"/zelena\" method=\"POST\"><input type=\"submit\" value=\"    Зелена СЕД    \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td><b><font color=#ff471a>" + zelena_LED_stanje + "</font><b/></td>"
              "</tr> "
              "<tr>"
              "<td><form action=\"/crvena\" method=\"POST\"><input type=\"submit\" value=\"    Црвена СЕД   \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td><b><font color=#ff471a>" + crvena_LED_stanje + "</font><b/></td>"
              "</tr>"
              "<tr>"
              "<td><form action=\"/zuta\" method=\"POST\"><input type=\"submit\" value=\"      Жута СЕД     \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td><b><font color=#ff471a>" + zuta_LED_stanje + "</font><b/></td>"
              "</tr>"
              "<tr>"
              "<td><form action=\"/usve\" method=\"POST\"><input type=\"submit\" value=\"  УКЉ. све СЕД  \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              " <td>xx</td>"
              "</tr>"
              "<tr>"
              " <td><form action=\"/isve\" method=\"POST\"><input type=\"submit\" value=\"  ИСК.  све СЕД  \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td>xx</td>"
              "</tr>"
              "<tr>"
              "<th colspan=\"2\">ATMEGA328P</th>"
              "</tr>"
              "<tr>"
              "<td><form></form></td>"
              "<td></td>"
              "</tr>"
              "<td><form action=\"/usve_atmega\" method=\"POST\"><input type=\"submit\" value=\"  УКЉ. све СЕД  \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td><b><font color=#ff471a>" + LED_stanje_atmega_sveu + "</font><b/></td>"
              "</tr>"
              "<tr>"
              " <td><form action=\"/isve_atmega\" method=\"POST\"><input type=\"submit\" value=\"  ИСК.  све СЕД  \"><text>&#160   </text><meta name=\"viewport\" content=\"width=device-width\" /></form></td>"
              "<td><b><font color=#ff471a>" + LED_stanje_atmega_svei + "</font><b/></td>"
              "</tr>"
              "</table></form></body></html>");
};
//Funkcije koje upravljaju radom SED
void serv_odg_plava() {
  p_stanje = digitalRead(13);
  if (p_stanje == "1")
    plava_LED_stanje = "ИС";
  else if (p_stanje == "0")
    plava_LED_stanje =  "УК";

  digitalWrite(13, !digitalRead(13));
  ekran_pisi("Plava SED", "promena stanja!");
  server.sendHeader("Location", "/");
  server.send(303);
};
void serv_odg_zelena() {
  ze_stanje = digitalRead(12);
  if (ze_stanje == "1")
    zelena_LED_stanje = "ИС";
  else if (ze_stanje == "0")
    zelena_LED_stanje =  "УК";

  digitalWrite(12, !digitalRead(12));
  ekran_pisi("Zelena SED", "promena stanja!");
  server.sendHeader("Location", "/");
  server.send(303);
};
void serv_odg_crvena() {
  c_stanje = digitalRead(16);
  if (c_stanje == "1")
    crvena_LED_stanje = "ИС";
  else if (c_stanje == "0")
    crvena_LED_stanje =  "УК";
   ekran_pisi("Crvena SED", "promena stanja!");
  digitalWrite(16, !digitalRead(16));
  server.sendHeader("Location", "/");
  server.send(303);
};
void serv_odg_zuta() {
  zu_stanje = digitalRead(0);
  if (zu_stanje == "1")
    zuta_LED_stanje = "ИС";
  else if (zu_stanje == "0")
    zuta_LED_stanje =  "УК";

  digitalWrite(0, !digitalRead(0));
  ekran_pisi("Zuta SED", "promena stanja!");
  server.sendHeader("Location", "/");
  server.send(303);
};
void serv_odg_usve() {
  plava_LED_stanje =  "УК";
  zelena_LED_stanje = "УК";
  crvena_LED_stanje = "УК";
  zuta_LED_stanje =  "УК";

  digitalWrite(16, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(0, HIGH);

  ekran_pisi("Sve SED", "ukljucene!");
  server.sendHeader("Location", "/");
  server.send(303);
};
void serv_odg_isve() {
  plava_LED_stanje =  "ИС";
  zelena_LED_stanje = "ИС";
  crvena_LED_stanje = "ИС";
  zuta_LED_stanje =  "ИС";

  digitalWrite(16, LOW);
  digitalWrite(13, LOW);
  digitalWrite(12, LOW);
  digitalWrite(0, LOW);

  ekran_pisi("Sve SED", "iskljucene!");
  server.sendHeader("Location", "/");
  server.send(303);
};
//Fje kojim se salje zahtev preko UART veze ka periferijskom uredjaju i osluskuje njegov odgovor
void serv_odg_usve_atmega() {
  soft_ser.write(1);
  soft_ser_proveri();
  server.sendHeader("Location", "/");
  server.send(303);
};
void serv_odg_isve_atmega() {
  soft_ser.write(2);
  soft_ser_proveri();
  server.sendHeader("Location", "/");
  server.send(303);
};

void serv_odg_nijeNadjeno()
{
  server.send(404, "text\plain", "404: Stranica nije pronadjena.");
};
//Ostale fje
//Funkcija kojom se podatak preuzima od periferijskog uredjaja i na 
//osnovu koje se menjaju stanja promenljivih u kojima cuvamo status SED
void soft_ser_proveri() {
    podatak = soft_ser.read();
    if (podatak == 3) {
      LED_stanje_atmega_sveu = 1;//"ИС";
      LED_stanje_atmega_svei = 0;//"УК";
      ekran_pisi("ATMEGA328P:", "Zelene SED uklj.");
      }
      else if (podatak == 4) {
      LED_stanje_atmega_sveu = 0; "УК";
      LED_stanje_atmega_svei = 1;//"ИС";
      ekran_pisi("ATMEGA328P:", "Zelene SED isklj");
      }
	  else if (podatak == greska_ATM) ekran_pisi("ATMEGA328P:", "GRESKA");
      
}
//Funkcija kojom se vrsi ispis korisnickih poruka na LCD ekranu
void ekran_pisi(String poruka0, String poruka1) {
 
  ekran.clear();
  ekran.setCursor(0, 0);
  ekran.print(poruka0);
  ekran.setCursor(0, 1);
  ekran.print(poruka1);
}
//##########################################################################################################################
void setup() {
  ekran.begin(16, 2);
  soft_ser.begin(9600);//Zapocni softversku serijsku vezu brzine 9.600 b/s ka periferijskom uredjaju
  Serial.begin(115200); //Zapocni serijsku vezu brzine 115.200 b/s ka racunaru
  delay(10); //cekaj 10 ms da se zapocne serijska veza
  Serial.println('\n');

  //Izvrsni elementi
  pinMode(16, OUTPUT); //crvena LED
  pinMode(0, OUTPUT);//zuta LED
  pinMode(13, OUTPUT); //plava LED
  pinMode(12, OUTPUT); //zelena LED

  //Pokretanje servera...
  povezi_WLAN();
  pokreni_SERVER();
}
//##########################################################################################################################
//Glavna petlja programa
void loop() {

  server.handleClient(); //Osluskuj HTTP zahteve od klijenata
  soft_ser_proveri(); //Osluskuj ima li odgovora od periferijskog uredjaja
}
//##########################################################################################################################











