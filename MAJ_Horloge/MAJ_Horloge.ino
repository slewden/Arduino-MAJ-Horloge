//---------------------------------------------------------------------------------------------------
// 27/03/2020 : finalisation de la librairie Horloge pour gérer un module RTC sur port I2C
// 27/03/2020 : Le programme sert aussi à mettre à l'heure le module en communiquant avec le PC 
//              ou en poussant l'heure depuis le moniteur serie (format dd/MM/yyyy HH:mm:ss)
//---------------------------------------------------------------------------------------------------
//--- Module RTC 4 fils : SDL -> A4  SDA -> A5 + GND et 5V : Adresse I2C : 0x68 ou 0x50 ?
//---------------------------------------------------------------------------------------------------
//--- le programme interroge le module RTC et renvoie la date de l'heure sur le port série
//--- si on envoie une chaine date valide au format dd/MM/yyyy HH:mm:ss sur le port série l'heure du 
//--- module RTC est mise à jour avec l'information.
//---------------------------------------------------------------------------------------------------
//--- Si vous voulez ajuster à partir de l'heure du PC : 
//---    Câblez un bouton sur l'arduino (entre la pin 2 et la masse GND)
//---    Lancez l'application "AdjutDateTimeFormPC.exe" (.net Core 3.1)
//---    Un appuie court sur le bouton ==> met à jour l'heure du module RTC avec l'heure du PC
//---    Un appuie long sur le bouton pour quitter l'application sur PC.
//---------------------------------------------------------------------------------------------------
#include <SPI.h> 
#include <Adafruit_SSD1306.h>
#include "Horloge.h"
//---------------------------------------------------------------------------------------------------
#define ADRESSE_HORLOGE 0x68   // adresse I2C de la carte RTC
#define PIN_BUTTON 2           // Pin du bouton pour demander de forcer l'heure depuis le PC
#define DELAY_QUIT 3000        // Temps en ms pour quitter l'application PC
//---------------------------------------------------------------------------------------------------
//--- Variables
Horloge myhorloge = Horloge(ADRESSE_HORLOGE);  // pour accéder au module RTC

String inputString = "";    // Chaine reçue sur le port série à analyser
DateTime_t adjustTime;      // Heure reçue à ajuster
DateTime_t now;             // Heure courante
byte bt;                    // Etat du bouton       
boolean adjusting = false;  // Mode ajustement activable
unsigned long timer;        // Pour mesurer le temps d'appui sur le bouton du bouton
int cpt = 100;              // compteur pour pas affiche des infos tous les 100ms

//---------------------------------------------------------------------------------
//--- Setup
void setup(){
  pinMode(PIN_BUTTON, INPUT_PULLUP);
 
  Serial.begin(9600);
  Serial.println(F("Démarrage en cours..."));
  Serial.println(F("Initialisation de la connexion à module RTC..."));
  Wire.begin();
  Serial.println(F("Module RTC initialisé."));
  Serial.println(F("Appuie bref pour ajuster l'heure"));
  Serial.println(F("Appuie long (> 3S) pour quitter"));
}

//---------------------------------------------------------------------------------
//--- Loop
void loop() {
  bt = digitalRead(PIN_BUTTON);
  if (bt == LOW) { // bouton enfoncé
    if (adjusting && millis() - timer > DELAY_QUIT) { // press + de 3 secondes
      Serial.println(F("quit")); // demande au PC de quitter
      adjusting = false;
    } else {
      if (!adjusting){ // premier appuie 
        adjusting = true;
        timer = millis();
      }
    }
  } else { // bt relaché
    if (adjusting) { // premier relachement (après une détection d'un appuie sur le bouton !)
      adjusting = false;
      Serial.println(F("adjust"));  // demande au PC d'envoyer l'heure sur le port série
    }

    if(myhorloge.getDateTimeNow(&now)) { // elle est pas démarrée
     if (cpt > 20) { // pour pas afficher trop souvent (Ici tous les 2s 20 x 100ms)
      Serial.println(F("Erreur GetDateTime"));
      cpt = 0;
     } else {
      cpt++;
     }
    } else { 
      if (cpt > 10) { // pour pas afficher trop souvent (Ici tous les 1s 10 x 100ms)
        Serial.println(myhorloge.getFullDateTimeString(now)); 
        cpt = 0;
      } else {
        cpt++;
      }
    } 
    delay(100); // ralentir l'arduino
  }
}

//---------------------------------------------------------------------------------
//--- SerialEvent : réception de caractères depuis le port série ==> la date à ajuster
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') { // fin de chaine
      bool done = false;
      do { // Tant qu'on a plein de caractères on essaie d'analyser
        Serial.print(F("Analyse de '")); Serial.print(inputString); Serial.println(F("' :"));
        if (stringValide(inputString)) { // si ok adjustTime contient la bonne heure
          myhorloge.adjust(adjustTime);  
          Serial.print(F("Nouvelle date et heure : ")); Serial.println(myhorloge.getFullDateTimeString(adjustTime)); 
          delay(500);
          inputString = "";
          done = true;
        } else if (inputString.length() > 1) { // on grignote le premier caractère pour voir si la chaine est valide
          inputString = inputString.substring(1);
        } 
      }
      while(!done && inputString.length() >= 19);
    } else {  // pas de fin de chaine on concatène
     inputString += inChar;
     //Serial.print("Received "); Serial.println(inputString); 
    }
  }
}

//---------------------------------------------------------------------------------
//--- StringValide : analyse la chaine txt, met à jour adjustTime, et renvoie true 
//--- si tout est ok
//--- pour que tout soit ok il faut que txt soit de la forme dd/MM/yyyy HH:mm:ss
boolean stringValide(String txt) {
  //Serial.print("Analyse de : '");  Serial.print(txt); Serial.println("'...");
  if (txt.length() < 19)
  { // pas assez de chiffres
    Serial.print(F(" KO : Taille de la chaine invalide (< 19) : "));  Serial.println(txt.length());
    return false;
  }
  //--- dd
  String t = txt.substring(0, 2);
  //Serial.print("Day : '");  Serial.print(t); Serial.println("' : ?");
  if (!isValidNumber(t, 1, 31)) {
    return false;
  }
  adjustTime.days = t.toInt();
  if (txt[2] != '/') {
    Serial.println(F(" KO : Premier slash non trouvé "));
    return false;   
  }
  //--- MM
  t = txt.substring(3, 5);
  //Serial.print("Month : '");  Serial.print(t); Serial.println("' : ?");
  if (!isValidNumber(t, 1, 12)) {
    return false;
  }
  adjustTime.months = t.toInt();
  if (txt[5] != '/') {
    Serial.println(F(" KO : Second Slash non trouvé "));
    return false;   
  }
  if (adjustTime.days == 31) { // on vérifie qu'un jour à 31 est avec un mois qui accepte 31 jours
   if (adjustTime.months == 2 || adjustTime.months == 4 || adjustTime.months == 6 || adjustTime.months == 9 || adjustTime.months == 11) {
     Serial.println(F(" KO : Jour incompatible avec le mois"));
     return false;   
    }
  } else if (adjustTime.months == 2 && adjustTime.days > 29) { // 30 et 31 février interdit !
    Serial.println(F(" KO : Jour incompatible avec février"));
    return false;   
  }  
  //--- YYYY
  t = txt.substring(6, 10);
  //Serial.print("year : '");  Serial.print(t); Serial.println("' : ?");
  if (!isValidNumber(t, 2000, 2099)) { // on verra le prochain siècle !
    return false;
  }
  int theyear = t.toInt();
  adjustTime.year =  theyear - 2000;
  if (adjustTime.months == 2 && adjustTime.days == 29) {
    if (!((theyear % 400) == 0 || (((theyear % 4) == 0) &&  ((theyear %100) != 0)))) {
      Serial.println(F(" KO : l'année n'est pas bisextile 29 fev interdit"));
      return false;   
    }
  }
  if (txt[10] != ' ') {
    Serial.println(F(" KO : Espace non trouvé "));
    return false;   
  }
  //--- HH
  t = txt.substring(11, 13);
  //Serial.print("Hour : '");  Serial.print(t); Serial.println("' : ?");
  if (!isValidNumber(t, 0, 23)) { 
    return false;
  }
  adjustTime.hours = t.toInt();
  if (txt[13] != ':') {
    Serial.println(F(" KO : Premier 2 points non trouvé "));
    return false;   
  } 
  //--- mm
  t = txt.substring(14, 16);
  //Serial.print("Minute : '");  Serial.print(t); Serial.println("' : ?");
  if (!isValidNumber(t, 0, 59)) { 
    return false;
  }
  adjustTime.minutes = t.toInt();
  if (txt[16] != ':') {
    Serial.println(F(" KO : Second 2 points non trouvé "));
    return false;   
  }
  //--- ss
  t = txt.substring(17, 19);
  //Serial.print("Second : '");  Serial.print(t); Serial.println("' : ?");
  if (!isValidNumber(t, 0, 59)) { 
    return false;
  }
  adjustTime.seconds = t.toInt();
  return true;
 }

//---------------------------------------------------------------------------------
//--- IsValidNumber : Vérifie que txt est est une string qui contient un int compris entre vmin(inclu) et vmax(inclu)
boolean isValidNumber(String txt, int vmin, int vmax) {
  int nb = txt.toInt();
  if (nb == 0 && txt != "0" && txt != "00") {
    Serial.print(F(" KO : '")); Serial.print(txt); Serial.print(F("' n'est pas un nombre valide entre ")); 
    Serial.print(vmin);  Serial.print(F(" et "));  Serial.println(vmax); 
    return false;
  }
  if (nb < vmin) {
    Serial.print(F(" KO : '")); Serial.print(txt); Serial.print(F("' est trop petit par rapport à "));
    Serial.println(vmin); 
    return false;
  }
  if (nb > vmax) {
    Serial.print(F(" KO : '")); Serial.print(txt); Serial.print(F("' est trop grand par rapport à "));
    Serial.println(vmax); 
    return false;
  }
  return true;
}
