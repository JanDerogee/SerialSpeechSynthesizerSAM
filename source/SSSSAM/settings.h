#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/*---------------------------------------------*/

#define SETTINGSFILENAME  "/settings.json"    /*this is the name of the configuration file where all settings are stored*/
#define JSONDOCSIZE       512             /*the size of the JSON document buffer*/

#define WELCOME_MSG_SIZE  160         /*the reset/welcome message has a maximum length*/
#define NETWORK_CFG_SIZE  32          /*the configuration strings of the network SSID and PASSWORD has a maximum length*/
#define CHECKSUM_OFFSET   0xCA        /*the checksum is offset by a certian value to make sure that we detect the empty flash (which may be 0x00 or 0xff the first time we use it)*/

#define DEFAULT_SPEED     72
#define DEFAULT_PITCH     64
#define DEFAULT_THROAT    128
#define DEFAULT_MOUTH     128
#define DEFAULT_SINGMODE  false
#define DEFAULT_PHONETIC  false
#define DEFAULT_PAUSE     30              /*additional pause of .. times 10 msec between sentences, this value is pleasant for use with scott adams adventures*/
#define DEFAULT_DEBUGMODE 1 //0           /*0=silent, 1=pronounce the configurations settings*/
#define DEFAULT_MSG1      "ready."        /*this is what SAM says when the system is reset (or powered on)*/
#define DEFAULT_MSG2      "ready."        /*this is what SAM says when the system is reset (or powered on)*/
#define DEFAULT_SSID      "YourNetwork"   /*this is the SSID of the network used for firmware updates*/
#define DEFAULT_PASS      "YourPassword"  /*this is the password for the network used for firmware updates*/
#define DEFAULT_TNETHOST  "YourTelnetHost"  /*this is the name of the telnet server we can connect to*/
#define DEFAULT_TNETPORT  "YourTelnetPort"  /*this is the port of the telnet server we can connect to*/
#define DEFAULT_SERIALDEL "5"               /*this is an additional delay of ... chars after the sending of a char. This in order to allow the CBM to process the data using regular BASIC without data overflow*/
#define DEFAULT_DICTIONARY true           /*this flag indicates if the dictionary function should be used*/

/*---------------------------------------------*/

/*a simple struct to hold all settings (that are stored in the JSON file)*/
typedef struct
{
  unsigned char spd       = DEFAULT_SPEED;
  unsigned char pitch     = DEFAULT_PITCH;
  unsigned char throat    = DEFAULT_THROAT;
  unsigned char mouth     = DEFAULT_MOUTH;
  unsigned char singmode  = DEFAULT_SINGMODE;
  unsigned char phonetic  = DEFAULT_PHONETIC;
  unsigned char pause     = DEFAULT_PAUSE;
  unsigned char debugmode = DEFAULT_DEBUGMODE;
  String        msg1      = DEFAULT_MSG1;  
  String        msg2      = DEFAULT_MSG2;  
  String        ssid      = DEFAULT_SSID;
  String        pass      = DEFAULT_PASS;
  String        tnethost  = DEFAULT_TNETHOST;
  String        tnetport  = DEFAULT_TNETPORT;
  String        serialdel = DEFAULT_SERIALDEL;
  unsigned char dictionary= DEFAULT_DICTIONARY;

//  String ntp = "time.nist.gov";     /*default value should be entered here (must be const char* otherwise it will not work with json related code)*/
//  float offset = 0;                 /*default value should be entered here*/
//  bool dst = false;                 /*default value should be entered here*/
//  bool alarm = true;                /*default value should be entered here*/
//  bool chime = true;                /*default value should be entered here*/

}settings_structTYPE;

extern settings_structTYPE settings_struct;  /*structure holding all the settings that should be available to all callers who includes this .h file*/


/*---------------------------------------------*/

bool SettingsLoad(void);                   /*load settings from JSON configuration file*/
bool SettingsSave(void);                   /*save settings to JSON configuration file*/
void SettingsDefault(void);

/*---------------------------------------------*/

#endif
