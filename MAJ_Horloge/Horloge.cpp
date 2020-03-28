//---------------------------------------------------------------------------------------------------
//--- librairie pour la gestion de l'horloge (Lire et ajuster l'heure sur le module RTC
//---------------------------------------------------------------------------------------------------
// 27/03/2020 : Maj ajout de fonctions pour afficher les secondes
//---------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <Wire.h>
#include "Horloge.h"

//---------------------------------------------------------------------------------------------------
//--- Horloge : contructeur : donne l'adresse du module
Horloge::Horloge(uint8_t a)
{
  this->_adresse = a;
}

//---------------------------------------------------------------------------------------------------
//--- Horloge : Fonction récupérant l'heure et la date courante à partir du module RTC.
//--- Place les valeurs lues dans la structure passée en argument (par pointeur).
//--- N.B. Retourne 1 si le module RTC est arrété (plus de batterie, horloge arrétée manuellement, etc.), 0 le reste du temps.
byte Horloge::getDateTimeNow(DateTime_t *datetime) {
  Wire.beginTransmission(this->_adresse); // Début de la transaction I2C
  Wire_write((byte) 0);             // Ecriture mémoire à l'adresse 0x00
  Wire.endTransmission();           // Fin de la transaction I2C
  Wire.requestFrom(this->_adresse, (byte) DS1307_CTRL_REGISTER); // Lit 7 octets depuis la mémoire du module RTC
  byte raw_seconds = Wire_read();
  datetime->seconds = this->bcd_to_decimal(raw_seconds);
  datetime->minutes = this->bcd_to_decimal(Wire_read());
  byte raw_hours = Wire_read();
  if (raw_hours & 64) { // Format 12h
    datetime->hours = this->bcd_to_decimal(raw_hours & 31);
    datetime->is_pm = raw_hours & 32;
  } else { // Format 24h
    datetime->hours = this->bcd_to_decimal(raw_hours & 63);
    datetime->is_pm = 0;
  }
  datetime->day_of_week = this->bcd_to_decimal(Wire_read());
  datetime->days = this->bcd_to_decimal(Wire_read());
  datetime->months = this->bcd_to_decimal(Wire_read());
  datetime->year = this->bcd_to_decimal(Wire_read());

  // Si le bit 7 des secondes == 1 : le module RTC est arrété
  return raw_seconds & 128;
}

//---------------------------------------------------------------------------------------------------
//--- Horloge : Fonction de conversion BCD -> decimal
byte Horloge::bcd_to_decimal(byte bcd) {
  return (bcd / 16 * 10) + (bcd % 16);
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : Fonction de conversion decimal -> BCD
byte Horloge::decimal_to_bcd(byte decimal) {
  return (decimal / 10 * 16) + (decimal % 10);
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : indique si deux dates sont identiques
byte Horloge::dateEquals(DateTime_t d1, DateTime_t d2) {
  if (d1.year != d2.year) {
    return false;
  }
  if (d1.months != d2.months) {
    return false;
  }
  if (d1.days != d2.days) {
    return false;
  }
  if (d1.hours != d2.hours) {
    return false;
  }
  if (d1.minutes != d2.minutes) {
    return false;
  }
  if (d1.seconds != d2.seconds) {
   return false;
  }
  return true;
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : Fonction ajustant l'heure et la date courante du module RTC à partir des informations fournies.
//--- N.B. Redémarre l'horloge du module RTC si nécessaire.
void Horloge::adjust(DateTime_t ladate) {
  Wire.beginTransmission(this->_adresse);                 // Début de la transaction I2C
  Wire_write((byte) 0);                             // Ecriture mémoire à l'adresse 0x00
  Wire_write(this->decimal_to_bcd(ladate.seconds) & 127); // CH = 0
  Wire_write(this->decimal_to_bcd(ladate.minutes));
  Wire_write(this->decimal_to_bcd(ladate.hours) & 63);    // Mode 24h
  Wire_write(this->decimal_to_bcd(ladate.day_of_week));
  Wire_write(this->decimal_to_bcd(ladate.days));
  Wire_write(this->decimal_to_bcd(ladate.months));
  Wire_write(this->decimal_to_bcd(ladate.year));
  Wire.endTransmission();                           // Fin de transaction I2C
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : Renvoie une chaine donnant la date et l'heure complète (format dd/MM/yyyy HH:mm:ss)
String Horloge::getFullDateTimeString(DateTime_t ladate) {
  String text = "";
  text += getDateString(ladate); 
  text += " ";  
  text += getTimeSecondString(ladate, 1); 
  return text;
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : Renvoie une chaine donnant la date (format dd/MM/yyyy)
String Horloge::getDateString(DateTime_t ladate) {
  String date = "";
  if (ladate.days < 10) {
    date += "0";
  }
  date += ladate.days;
  date += "/";
  if (ladate.months < 10) {
    date += "0";
  }
  date += ladate.months;
  date += "/";
  date += (ladate.year + 2000);
  return date;
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : Renvoie une chaine donnant l'heure et le minutes (format HH:mm)
String Horloge::getTimeString(DateTime_t ladate, byte sepi) {
  String text = "";
  if (ladate.hours < 10) {
    text += "0";
  }
  text += ladate.hours;
  text += sepi == 1 ? ":" : " ";
  if (ladate.minutes < 10) {
    text += "0";
  }
  text += ladate.minutes;
  return text;
}
//---------------------------------------------------------------------------------------------------
//--- Horloge : Renvoie une chaine donnant l'heure, minutes et les secondes (format HH:mm:ss)
String Horloge::getTimeSecondString(DateTime_t ladate, byte sepi) {
  String text = "";
  text += getTimeString(ladate, sepi);
  text += sepi == 1 ? ":" : " ";
  if (ladate.seconds < 10) {
    text += "0";
  }
  text += ladate.seconds;
  return text;
}
