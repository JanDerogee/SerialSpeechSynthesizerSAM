#include <Arduino.h>      /*important to avoid all sorts of strange error messages*/
#include <ArduinoJson.h>  /*this can be installed using the arduino library manager, it is in the list, but not installed by default*/
#include <FS.h>
#include "settings.h"

/*=======================================================================================*/
/*                                 v a r i a b l e s                                     */
/*=======================================================================================*/

settings_structTYPE settings_struct;  /*structure holding all the settings that should be available to all callers who includes this .h file*/

/*=======================================================================================*/
/*                                  r o u t i n e s                                      */
/*=======================================================================================*/

/*This routine will restore the current settigs to the default values*/
/*the settings are stored in non volatile memory are not changed, if the user desires this*/
/*the he/she needs to issue the save-command*/
void SettingsDefault(void)
{
  settings_struct.spd        = DEFAULT_SPEED;
  settings_struct.pitch      = DEFAULT_PITCH;
  settings_struct.throat     = DEFAULT_THROAT;
  settings_struct.mouth      = DEFAULT_MOUTH;
  settings_struct.singmode   = DEFAULT_SINGMODE;
  settings_struct.phonetic   = DEFAULT_PHONETIC;  
  settings_struct.pause      = DEFAULT_PAUSE;
  settings_struct.debugmode  = DEFAULT_DEBUGMODE;
  settings_struct.msg1       = DEFAULT_MSG1;
  settings_struct.msg2       = DEFAULT_MSG2;  
  settings_struct.ssid       = DEFAULT_SSID;
  settings_struct.pass       = DEFAULT_PASS;
  settings_struct.tnethost   = DEFAULT_TNETHOST;
  settings_struct.tnetport   = DEFAULT_TNETPORT;
  settings_struct.serialdel  = DEFAULT_SERIALDEL;  
  settings_struct.dictionary = DEFAULT_DICTIONARY; 
}

/*load settings from JSON configuration file*/
bool SettingsLoad(void)
{ 
  char c_string[WELCOME_MSG_SIZE+1];  /*general purpose string used for printing of text and such*/
 //SPIFFS.begin();   
  File configFile = SPIFFS.open(SETTINGSFILENAME, "r"); /*Open file for reading*/
  if (!configFile)
  {
    Serial.println(F("Failed to open config file, attempting to create file"));
    SettingsSave();
    return(false);
  }

  /*debug only*/
  Serial.println("Contents of settingsfile");
  while (configFile.available())  /*Extract each characters by one by one*/
  {
    Serial.print((char)configFile.read());
  }
  Serial.println();  
  Serial.println();  
  configFile.close();
  
  configFile = SPIFFS.open(SETTINGSFILENAME, "r"); 
  size_t size = configFile.size();
  if (size > JSONDOCSIZE)
  {
    Serial.println(F("Config file size is too large"));
    return(false);
  }

  
  StaticJsonDocument<JSONDOCSIZE> doc;  /* Allocate a temporary JsonDocument. Don't forget to change the capacity to match your requirements. Use arduinojson.org/v6/assistant to compute the capacity.*/
  DeserializationError error = deserializeJson(doc, configFile);                /*Deserialize the JSON document*/
  if(error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }

  Serial.println(F("Loading settings"));
  /*copy all values from the JSON file into the proper variables*/

  settings_struct.spd       = doc["spd"]      | DEFAULT_SPEED;
  settings_struct.pause     = doc["pitch"]    | DEFAULT_PITCH;
  settings_struct.throat    = doc["throat"]   | DEFAULT_THROAT;
  settings_struct.mouth     = doc["mouth"]    | DEFAULT_MOUTH;
  settings_struct.singmode  = doc["singmode"] | DEFAULT_SINGMODE;
  settings_struct.phonetic  = doc["phonetic"] | DEFAULT_PHONETIC;
  settings_struct.pause     = doc["pause"]    | DEFAULT_PAUSE;
  settings_struct.debugmode = doc["debugmode"]| DEFAULT_DEBUGMODE;

         /*destination          source   default value destination's capacity*/
  strlcpy(c_string, doc["msg1"] | DEFAULT_MSG1, WELCOME_MSG_SIZE);
  settings_struct.msg1 = c_string;
  strlcpy(c_string, doc["msg2"] | DEFAULT_MSG2, WELCOME_MSG_SIZE);
  settings_struct.msg2 = c_string;
  strlcpy(c_string, doc["ssid"] | DEFAULT_SSID, NETWORK_CFG_SIZE);
  settings_struct.ssid = c_string;
  strlcpy(c_string, doc["pass"] | DEFAULT_PASS, NETWORK_CFG_SIZE);
  settings_struct.pass = c_string;
  strlcpy(c_string, doc["tnethost"] | DEFAULT_TNETHOST, WELCOME_MSG_SIZE);
  settings_struct.tnethost = c_string; 
  strlcpy(c_string, doc["tnetport"] | DEFAULT_TNETPORT, WELCOME_MSG_SIZE);
  settings_struct.tnetport = c_string;   

  settings_struct.serialdel = doc["serialdel"]| DEFAULT_SERIALDEL;

  settings_struct.dictionary= doc["phonetic"] | DEFAULT_DICTIONARY;
 
  return(true);
}

/*save settings to JSON configuration file*/
bool SettingsSave(void)
{  
  StaticJsonDocument<JSONDOCSIZE> doc;  /*Allocate a temporary JsonDocument. Don't forget to change the capacity to match your requirements. Use arduinojson.org/assistant to compute the capacity.*/

  /*copy variables to document*/
  doc["spd"]      = settings_struct.spd;
  doc["pitch"]    = settings_struct.pitch;
  doc["throat"]   = settings_struct.throat;
  doc["mouth"]    = settings_struct.mouth;
  doc["singmode"] = settings_struct.singmode;  
  doc["phonetic"] = settings_struct.phonetic;  
  doc["pause"]    = settings_struct.pause;  
  doc["debugmode"]= settings_struct.debugmode;  
  doc["msg1"]     = settings_struct.msg1;
  doc["msg2"]     = settings_struct.msg2;  
  doc["ssid"]     = settings_struct.ssid;  
  doc["pass"]     = settings_struct.pass;    
  doc["tnethost"] = settings_struct.tnethost;    
  doc["tnetport"] = settings_struct.tnetport;   
  doc["serialdel"]= settings_struct.serialdel;    
  
  File configFile = SPIFFS.open(SETTINGSFILENAME, "w");
  if (!configFile)
  {
    Serial.println(F("Failed to open config file for writing"));
    return(false);
  }
  else
  {
    
    //if (serializeJson(doc, configFile) == 0)    /*Serialize JSON to file*/
    if (serializeJsonPretty(doc, configFile) == 0)    /*Serialize JSON to file, but in a more human readable form*/
    {
      Serial.println(F("Failed to write to file"));
    }
  }

  doc["dictionary"] = settings_struct.dictionary;    

  return(true);
}
