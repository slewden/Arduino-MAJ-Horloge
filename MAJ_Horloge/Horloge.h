//---------------------------------------------------------------------------------------------------
//--- librairie pour la gestion de l'horloge (Lire et ajuster l'heure sur le module RTC
//---------------------------------------------------------------------------------------------------
// 27/03/2020 : Maj ajout de fonction pour afficher les secondes
//---------------------------------------------------------------------------------------------------
#ifndef horlogeModule
#define horlogeModule

#include <Arduino.h>

//---------------------------------------------------------------------------------------------------
// Rétro-compatibilité avec Arduino 1.x et antérieur
#if ARDUINO >= 100
  #define Wire_write(x) Wire.write(x)
  #define Wire_read() Wire.read()
#else
  #define Wire_write(x) Wire.send(x)
  #define Wire_read() Wire.receive()
#endif

//---------------------------------------------------------------------------------------------------
//---  Structure DateTime_t : contenant les informations de date et heure en provenance ou 
//---  à destination du module RTC 
typedef struct {
  uint8_t seconds;      /* Secondes 00 - 59 */
  uint8_t minutes;      /* Minutes 00 - 59 */
  uint8_t hours;        /* Heures 00 - 23 (format 24h), 01 - 12 (format 12h) */
  uint8_t is_pm;        /* Vaut 1 si l'heure est en format 12h et qu'il est l'apr�s midi, sinon 0 */
  uint8_t day_of_week;  /* Jour de la semaine 01 - 07, 1 = lundi, 2 = mardi, etc.  */
  uint8_t days;         /* Jours 01 - 31 */
  uint8_t months;       /* Mois 01 - 12 */
  uint8_t year;         /* Ann�e au format yy (exemple : 16 = 2016) */
} DateTime_t;


//---------------------------------------------------------------------------------------------------
//--- Constantes pour le module RTC
//-- Adresse du registre de contrôle du module RTC DS1307 
const uint8_t DS1307_CTRL_REGISTER = 0x07; 
//-- Adresse et taille de la NVRAM du module RTC DS1307 
const uint8_t DS1307_NVRAM_BASE = 0x08;
const uint8_t DS1307_NVRAM_SIZE = 56;
//--- Mode de fonctionnement pour la broche SQW 
typedef enum {
  SQW_1_HZ = 0, // Signal à 1Hz sur la broche SQW 
  SQW_4096_HZ,  // Signal à 4096Hz sur la broche SQW
  SQW_8192_HZ,  // Signal à 8192Hz sur la broche SQW
  SQW_32768_HZ, // Signal à 32768Hz sur la broche SQW 
  SQW_DC        // Broche SQW toujours à LOW ou HIGH 
} DS1307_Mode_t;

//--- classe pour l'accès au module RTC
class Horloge
{
  public:
    //--- Horloge : contructeur : donne l'adresse du module
    Horloge(uint8_t a);
   
    //--- Horloge : Fonction récupérant l'heure et la date courante à partir du module RTC.
    //--- Place les valeurs lues dans la structure passée en argument (par pointeur).
    //--- N.B. Retourne 1 si le module RTC est arrété (plus de batterie, horloge arrétée manuellement, etc.), 0 le reste du temps.
    byte getDateTimeNow(DateTime_t *datetime);

    //--- Horloge : Fonction ajustant l'heure et la date courante du module RTC à partir des informations fournies.
    //--- N.B. Redémarre l'horloge du module RTC si nécessaire.
    //--- N.B. Force un format 24h
    void adjust(DateTime_t ladate);

    //--- Horloge : indique si deux structures dates sont identiques
    byte dateEquals(DateTime_t d1, DateTime_t d2);

    //--- Horloge : Renvoie une chaine donnant la date et l'heure complète (format dd/MM/yyyy HH:mm:ss)
    String getFullDateTimeString(DateTime_t ladate);

    //--- Horloge : Renvoie une chaine donnant la date (format dd/MM/yyyy)
    String getDateString(DateTime_t ladate);

    //--- Horloge : Renvoie une chaine donnant l'heure et le minutes (HH:mm)
    String getTimeString(DateTime_t ladate, byte sepi);

    //--- Horloge : Renvoie une chaine donnant l'heure, minutes et les secondes (format HH:mm:ss)
    String getTimeSecondString(DateTime_t ladate, byte sepi);
  private:
   byte bcd_to_decimal(byte bcd);
   byte decimal_to_bcd(byte decimal);
   uint8_t _adresse;
};


#endif // horlogeModule
// Fin du module horloge 
