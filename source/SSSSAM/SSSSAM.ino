/*Make sure the following settings in the arduino IDE are used
 * 
 * build using core 2.7.4, JSON version 6
 * 
  Tools:
    board: ESP 12E
    uploadspeed: 115200
    CPU freq.: 160MHz
    flash size: 4M (1M SPIFFS)
    Debug port: disabled              \___ disabled and noassert saves both RAM and flash and prevent annoying messages from the core
    Debug level: NoAssert-NDEBUG      /
    IwIp: v2 lower memory
    Vtables: flash
    Exceptions: disabled
    Erase flash: only sketch


In order to get the SPIFFS.BIN you just compile your program (whith the spiffs related files in the data folder of your project folder
Program your device using the arduino upload function and upload your spiffs using the arduino spiffs upload tool, then you'll quickly
see in the Arudino terminal output, something along the lines of:
[SPIFFS] upload : C:\Users\USER\AppData\Local\Temp\build87a71177855ef0141b9b3758467a3f0f.spiffs/Project.spiffs.bin
Within that path you'll find the .BIN file you'll require
    
*/

/* This code uses the SAM speech synthesizer EPS8266 port as done by: Earle F. Philhower 
 *  
 * Software Automatic Mouth (SAM) was an amazing speech synthesizer available on 8-bit CPUs in the early 80s.
 * There were versions for the Atari 400, Commodore 64, and others. A fan converted it from 6502 assembly to
 * C code and put it online ( https://github.com/s-macke/SAM ).
 * 
 * Then, some time later, Earle F. Philhower came along and ported it to the ESP8266 and made it very usable
 * by making a nice library for it and combining it with his ESP8266 audio library.
 * He took that code, reworked the output so it sent bytes directly to the audio device instead of buffering,
 * and moved what tables I could into PROGMEM. Therefore the memory requirements are very low.
 * In order to use it, you need both the audio- and the SAM-library as found on his github pages:
 * https://github.com/earlephilhower/ESP8266SAM
 * https://github.com/earlephilhower/ESP8266Audio
 * 
 * (in other words, you need to install these zip files) ESP8266SAM-master.zip, ESP8266Audio-master.zip, ESP8266_Spiram-master.ZIP
 * 
 * Then some time later, I came along (Jan Derogee) noticed this piece of very usable code and made a small 
 * serial port interface around it, so that it can be used like a device like the Votrax.
 * Then a nice PCB was also made for it, so it could be connected to a CBM computer, like the VIC-20.
 * This way it can be used like a Votrax speech synthesizer in combination with the Scott Adams adventure games.
 * Or your own programs if you'd like as using it is relatively easy. The fun thing is is that it takes up no
 * memory from your old computer as it is basically a speech co-processor connected via the serial port.
 * 
 * SAM (the Software Acoustic Mouth) was created by: "Don't ask Software" a long long time ago somewhere in the 80's
 * and fascinated me from the first moment I played with it when I was a youngster. These days speech synthesizers are
 * something "normal" (thanks to the mobile phone revolution). But back in the 80's it was cutting edge technology
 * promissing a brighter future. The library written by Earle in combination with my small serial interface should make
 * it very easy to add this to any old computer. As it does not take up processing power, it is dirt cheap and the code
 * required on the old computer is nothing more then some print statements to the serial port. Which can be programmed 
 * in basic quite easily.
 * 
 * 
 * send the following data over sthe serial port:
 * string                         result
 * ----------------------------------------------------------------
 * hello world.                   SAM speaks "hello world"
 * 
 * -demo                          play a simple demo, just to show of
 * 
 * -config show                   this shows all possible information, firmware version and settings
 * -config speed 128              speed value is set to 128   (value has to be within 0-255, values larger then 255 are not accepted and results in no change of the setting, spaces between keywords are not required, but improve readabillity)
 * -config pitch 128              pitch value is set to 128
 * -config mouth 128              mouth value is set to 128
 * -config throat 128             throat value is set to 128
 * 
 * -config debug 1                debug=0 (default setting) no pronuncination of etting commands meaning that the system is completely silent with exception of normal speech
 *                                debug=1 prounce every setting command
 * -config singmode 1             singmode=0 (default setting) allows for a more natural sounding speech due to tone bending during pronuncination of the words
 *                                singmode=1 prevent tonebending during pronoucination of the text
 * -config phonetic 1             phonetic=0 (default setting) uses the text-to-speech layer (very easy but not always accurate pronuncination)
 *                                phonetic=1 allows for the use of phonemes (difficult but allows for more precise control)
 * -config pause 1                pause=1 means that a pause of 1*10ms will be waited after each sentence before a new sentence is spoken                               
 * -config msg1 <text>            this sets the text for welcome message (just replace <text> with you own personal text that is spoken after each cold-start (power-on)
 * -config msg2 <text>            this sets the text for welcome message (just replace <text> with you own personal text that is spoken after each warm-start (reset)
 * 
 * -config save                   this saves all current setting values to eeprom
 * -config load                   this re-loads all settings values from eeprom, so if you goofed up while changing settings (but haven't saved yet), this is the command you use without referring to the default settings
 * -config default                this command restores all settings back to the factory default settings
 * 
 * 
 * other experimental commands are : telnethost, telnetport, telnetstart, serialdelay
 * 
 * Combinations of settings are also possible, as this would allow for smaller code on the computer controlling this device
 * -config speed 100 pitch 128 etc.   
 * However..., this doesn't apply to the configuration os msg1 and msg2, because those are string commands and it is impossible for SAM
 * to detect when the string ends, so if you do concatenate you might not get what you expect.
 /////////// * -config show                   return all values
 */

/*----------------------------------------------------------------*/
/* You can pronunce the text MY NAME IS SAM in 2 different ways   */
/*   textmode      (phonetic=0):   MY NAME IS SAM                 */
/*   phonetic mode (phonetic=1):   MAY4 NEYM IHZ SAE4M            */
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* DESCRIPTION          SPEED     PITCH     THROAT    MOUTH       */
/*----------------------------------------------------------------*/
/* SAM (default)         72        64        128       128        */
/* Elf                   72        64        110       160        */
/* Little Robot          92        60        190       190        */
/* Stuffy Guy            82        72        110       105        */
/* Little Old Lady       82        32        145       145        */
/* Extra-Terrestrial    100        64        150       200        */
/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
/*                         I N C L U D E S                        */
/*----------------------------------------------------------------*/
#include <Arduino.h>
#include <ESP8266SAM.h>
#include <AudioOutputI2SNoDAC.h>    /*if we would have used a real I2S DAC (a real hardware DAC chip) we would have needed to include: #include <AudioOutputI2S.h>  */

#include "FS.h"           /*if someday we need spiffs, then we must include it from day one, otherwise we get compatibility issues between users*/
#include "settings.h"
#include "rtcusermem.h"   /*the RTC user mem is RAM unaffected by reset (perfect for short term storage*/
#include "FSBrowser.h"    /*browse through the files in the SPIFFS filesystem using an ordinary webbrowser*/

/*---*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
/*---*/

extern "C"
{
  #include <user_interface.h>
}    /* https://github.com/esp8266/Arduino actually tools/sdk/include */

/*----------------------------------------------------------------*/
/*                       CONSTANTS AND DEFINES                    */
/*----------------------------------------------------------------*/

#define LED_BUSY              4
//#define CBM_TXD             13
#define BB_TXD                5   /*this signal connects to CBM_RXD*/

#define RS32_BAUDRATE         115200        /*baudrate for debugging purposes*/
#define CBM_BAUDRATE          2400          /*baudrate fast enough for this purpose, but not to fast for the old 8-bit computer is is connected to*/
#define MAX_STRING_SIZE       160           /*internal string buffer size*/

#define EASTEREGG_THRESSHOLD  4000          /*the a certain value doesn't exceed this trigger some alternative actions are taken*/

/*The table below shows the values required to approximate the perfect note.                */
/*The speech synth isn't very accurate regarding pitch, the error get's worse when pitch    */
/*the pitch values get lower (freq higher). But the result is good enough for a simple song.*/
/*                                                                                          */
/*The small list of defines, describe the Notes over a range of 3 octaves (0,1 and 2), the N*/
/*in the define name is simply to keep the name unique and to prevent confusion with hex num*/
/*     Note  SAM pitch   Frequency (for the perfect note)*/
/*     --------------------------- */
#define N0C   115       //130.82
#define N0C_  108       //138.59
#define N0D   103       //146.83
#define N0D_  98        //155.56
#define N0E   94        //164.81
#define N0F   88        //174.61
#define N0F_  82        //185
#define N0G   78        //196
#define N0G_  74        //207.65
#define N0A   70        //220
#define N0A_  66        //233.08
#define N0B   62        //246.94

#define N1C   58        //261.63
#define N1C_  55        //277.18
#define N1D   52        //293.66
#define N1D_  49        //311.13
#define N1E   46        //329.63
#define N1F   44        //349.23
#define N1F_  42        //369.99
#define N1G   39        //392
#define N1G_  37        //415.3
#define N1A   35        //440
#define N1A_  33        //466.16
#define N1B   31        //493.88

#define N2C   29        //523.25
#define N2C_  28        //554.37
#define N2D   26        //587.33
#define N2D_  25        //622.25
#define N2E   23        //659.26
#define N2F   22        //698.46
#define N2F_  21        //739.99
#define N2G   20        //783.99
#define N2G_  19        //830.61
#define N2A   18        //880
#define N2A_  17        //932.33
#define N2B   16        //987.77


/*----------------------------------------------------------------*/
/*                         GLOBAL VARIABLES                       */
/*----------------------------------------------------------------*/

//AudioOutputI2S *out = NULL;       /*use this when a true I2S DAC is required, which in many cases isn't as the quality of the I2SNoDAC is very good*/
AudioOutputI2SNoDAC *out = NULL;    /*we use this because of the low cost as it does not require an I2S DAC module*/
ESP8266SAM *sam = new ESP8266SAM;   /*global, so we can use it everywhere, which might come in handy*/

File file;                          /*a simple filehandle for use with the SPIFFS (the internal filesystem of the ESP8266)*/

char c_string[WELCOME_MSG_SIZE+1];  /*general purpose string used for printing of text and such*/
char c2_string[WELCOME_MSG_SIZE+1];  /*general purpose string used for printing of text and such*/



/*a simple struct for dictionary usage*/
typedef struct
{
  unsigned char enabled;
  String filename;
  unsigned long index[26];
}dictionary_structTYPE;

dictionary_structTYPE primary_dict;     /*structure holding all the settings that should be available to all callers who includes this .h file*/
dictionary_structTYPE secondary_dict;   /*structure holding all the settings that should be available to all callers who includes this .h file*/

/*----------------------------------------------------------------*/
/*                      SUBROUTINE DECLARATIONS                   */
/*----------------------------------------------------------------*/

void dbg_message(char *msg);
unsigned char Process_string(char *str, unsigned char add_extra_pause);
unsigned char Check_keyword(char *str, char *keyword);
void Remove_processed(char *str, unsigned char len);
char PETSCII_to_ASCII(char c);
char ASCII_to_PETSCII(char c);

int Get_value(char *str);
void Get_string(char *str, char *result, unsigned char maxsize);
void Send_BBTXD_byte(char data);
void Send_BBTXD_string(char *str);
void ShowSettings(unsigned char destination);
void ConnectToWifi(void);
void StartUpdateServer(void);
void StartTelnetClient(void);
unsigned char Dictionary_index(dictionary_structTYPE *d);
unsigned char Dictionary_search(dictionary_structTYPE *d, char *searchword, char *phoneticword);
void Play_demo_1(void);

/*----------------------------------------------------------------*/
/*                        I N I T I A L I Z E                     */
/*----------------------------------------------------------------*/

void setup()
{
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();
  
  /*make sure all SPI related chipselect lines are defined before we attempt any SPI communication*/
  pinMode(LED_BUSY, OUTPUT);      /*init the busy LED IO-port*/  
  digitalWrite(LED_BUSY, LOW);    /*switch it off*/  
  
  digitalWrite(BB_TXD, HIGH);     /*set pin to the correct level (if we don't we get a glitch of the line briefly going low when set to output, which is undesired for this application)*/  
  pinMode(BB_TXD, OUTPUT);        /*init the busy LED IO-port (if it wasn't already)*/  
  digitalWrite(BB_TXD, HIGH);     /*generate idle situation*/

  Serial.begin(RS32_BAUDRATE);
  Serial.setDebugOutput(false); /*set this to "true" in order to get more information if the ESP8266 crashes but we don't know why...*/      
  Serial.setRxBufferSize(1024); /*define buffersize of serialport, if it is too small we'll be missing data during speech synth*/

  WiFi.disconnect();                /*only required when the Cassiopei is reset, this is not required when the cassiopei is power-on*/
  WiFi.persistent(false);           /*Do not memorise new wifi connections*/
  //ESP.flashEraseSector(0x3fe);    /*Erase remembered connection info. (Only do this once, this is more of a debug/development kind of thing).*/

  Serial.println("\r\n--== Serial Speech Synthesizer SAM ==--");
  Serial.println("Firmware version:" __DATE__ );

  /*some simple flash identification (from the example: CheckFlashConfig)*/
  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u bytes\n", realSize);
  Serial.printf("Flash ide  size: %u bytes\n", ideSize);
  Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
  if (ideSize != realSize)  {Serial.println("Flash Chip configuration wrong!");}
  else                      {Serial.println("Flash Chip configuration ok.");}
  
  Serial.print("Free heap size ="); /*show available RAM*/
  Serial.println(ESP.getFreeHeap()); /*show available RAM*/
  Serial.println("");

  if(!SPIFFS.begin())
  {
    Serial.println ("An Error has occurred while mounting SPIFFS");
    return;
  }

  Dir dir = SPIFFS.openDir ("");
  int total_file = 0;
  long total_size = 0;
  Serial.println("Directory of SPIFFS:");
  while(dir.next ())
  {
    total_file++;
    Serial.print("filename:");    
    Serial.print(dir.fileName());
    Serial.print("\t filesize:");
    total_size = total_size + dir.fileSize();
    Serial.println (dir.fileSize());
  }
  Serial.print("Total of ");
  Serial.print(total_file);
  Serial.print(" files using ");
  Serial.print(total_size);
  Serial.println(" bytes");
  Serial.println();

  SettingsLoad();     /*load the settings, like welcome message and voice related settings, from non volatile memory*/
  ShowSettings(0);  /*show the settings, send the data to the standard serial port (the one also used by the bootloader)*/

  primary_dict.filename = "/dict_pri.txt";
  secondary_dict.filename = "/dict_sec.txt";
  if(settings_struct.dictionary == true) /*check if dictionary is enabled*/
  {                         
    Dictionary_index(&primary_dict);
    Dictionary_index(&secondary_dict); /*when both indexing routines fail, disable the dictionary mode*/
  }
       
  delay(1000);  /*give the serial port routine some time to send the data before we swap the serial ports and prevent further communication*/

  Serial.begin(CBM_BAUDRATE);     /*change baudrate for use with old 8-bit computer*/
  Serial.swap();                  /*swap serial port, because the I2S lines are obstructing RXD signal path*/

  /*now we set the audio output, do not mess with the serial port settings (like .swap or .begin) as it will mess up the I2S configuration and we hear nothing*/
  //out = new AudioOutputI2S();
  out = new AudioOutputI2SNoDAC();  
  out->begin();

  /*Read struct from RTC memory, we use this method because the normal method of determination of resettype (ESP.getResetReason();) doesn't work on our board because of the absence of the required delay between the rising of enable and the rising of reset*/
  if(ReadPersistentStruct() == false)
  { /*cold start (user turned on the computer)*/
    settings_struct.msg1.toCharArray(c_string, (settings_struct.msg1.length()+1));  /*our variable is stored as a string, but we need a char array*/    
    sam->Say(out, c_string);
    rtcData.data[0] = 0;
    WritePersistentStruct();  /*update the struct that has the data unaffected by reset*/    
  }
  else
  { /*warm start (user presed reset)*/
    //sprintf(c_string,"rtcData.counter=%d", rtcData.counter);     /*debug only*/
    //Send_BBTXD_string(c_string);                                 /*debug only*/
    
    if(rtcData.counter >= EASTEREGG_THRESSHOLD) /*rtcDdata.counter isn't accurate at all but more then good enough for determining if somebody is pressing reset too quickly after eachother*/
    {
      settings_struct.msg2.toCharArray(c_string, (settings_struct.msg2.length()+1));  /*our variable is stored as a string, but we need a char array*/    
      sam->Say(out, c_string);
      rtcData.data[0] = 0;                      /*reset easter egg*/
    }
    else
    {
      switch(rtcData.data[0])
      {
        case 0: {sam->Say(out,"Reset button pressed");                                      rtcData.data[0]++;  break;}
        case 1: {sam->Say(out,"And now you pressed it again!");                             rtcData.data[0]++;  break;}
        case 2: {sam->Say(out,"What are you doing?");                                       rtcData.data[0]++;  break;}
        case 3: {sam->Say(out,"Please stop pressing reset.");                               rtcData.data[0]++;  break;}
        case 4: {sam->Say(out,"Hay, cut it out!");                                          rtcData.data[0]++;  break;}
        case 5: {sam->Say(out,"This is getting annoying!");                                 rtcData.data[0]++;  break;}
        case 6: {sam->Say(out,"STOP IT! Please?");                                          rtcData.data[0]++;  break;}
        case 7: {sam->Say(out,"If you press reset, once more, i won't speak to you again.");rtcData.data[0]++;  break;}        
        case 8: {sam->Say(out,"I mean it!");                                                rtcData.data[0]++;  break;}
        case 9: {sam->Say(out,"Okay, you asked for it!");                                   rtcData.data[0]++;  break;}
        case 10:{                                                                           rtcData.data[0]++;  break;}        
        case 11:{                                                                           rtcData.data[0]++;  break;}
        case 12:{sam->Say(out,"Seriously, you really must stop!");                          rtcData.data[0]++;  break;}        
        case 13:{sam->Say(out,"I don't want to play anymore!");                             rtcData.data[0]++;  break;}
        case 14:{sam->Say(out,"Good bye!");                                                 rtcData.data[0]++;  break;}        
        default:{sam->Say(out, "");  break;}
      }
    }
    
  }
  rtcData.counter = 0;
  WritePersistentStruct();                  /*update the struct that has the data unaffected by reset*/
  Send_BBTXD_string("\rok\r");              /*indicate that we are ready for new commands*/
}


/*----------------------------------------------------------------*/
/*                              M A I N                           */
/*----------------------------------------------------------------*/

void loop()
{
  char data_string[MAX_STRING_SIZE];  /*the CBM does not fit any more chars then 80 on a single line for the most expensive model*/
  char data_char;
  char prev_data_char;
  char pntr;

  pntr = 0;
  prev_data_char = 0;
  while(1)
  {
    delay(1); /*pet the watchdog*/

    if(rtcData.counter < EASTEREGG_THRESSHOLD)  /*a simple counter to keep track of the time between resets*/
    {
      rtcData.counter++;
      WritePersistentStruct();  /*update the struct that has the data unaffected by reset*/      
    }

    /*when we are about to overflow the buffer, there is something horribly wrong, best would be to ignore everything we have received so far and speak an error text*/
    if(pntr == MAX_STRING_SIZE)
    {
      sam->Say(out,"Error. serial buffer overflow.");
      pntr=0;
      data_char = 0;                      /*this will set prev_data_char to a non-space character further in the code*/
      Send_BBTXD_string("\rok\r");          /*indicate we are ready for a new command*/         
    }

    if(Serial.available() > 0)
    {           
      data_char = Serial.read();
      data_char = PETSCII_to_ASCII(data_char); /*convert the PETSCII chars of the CBM into ASCII values of the same meaning*/
      //data_char = tolower(data_char); /*make sure all characters are handled as lower case (we don't want SAM to shout) but seriously, it could make command processing a little bit easier*/
      //Send_BBTXD_byte(data_char);     /*echo everything that is send*/

      /*Substitue double spaces by a CR (scott adams games, send double spaces where a CR would be required)*/
      if((data_char == ' ') and (prev_data_char == ' '))
      {
        data_char = 13;
      }

      switch(data_char)
      {
        case 10:  /*check for LF*/
        case 13:  /*check for CR*/
        {
          data_string[pntr]=0;                /*add end of string marker*/
          pntr = 0;                           /*reset pointer, so this effectively reset the string for the next itteration*/
          data_char = 0;                      /*this will set prev_data_char to a non-space character further in the code*/          
          Process_string(data_string, true);  /*process string and add a pause to it if it was a spoken sentence*/
          Send_BBTXD_string("\rok\r");          /*indicate we are ready for a new command*/          
          break;
        }

        case '.': /*check for end of sentence*/
        case '!': /*check for end of sentence*/
        case '?': /*check for end of sentence*/
        {
          data_string[pntr]=data_char;      /*add char to string*/
          pntr++;                           /*increment pointer*/
          if(data_string[0] != '-')         /*when the string starts with a - it may very likely contain a config command, in that case we must wait out for the entire string before we process it*/
          {
            data_string[pntr]=0;              /*add end of string marker*/
            pntr = 0;                         /*reset pointer*/
            data_char = 0;                      /*this will set prev_data_char to a non-space character further in the code*/            
            Process_string(data_string, true);  /*process string and add a pause to it if it was a spoken sentence*/
          }
          break;
        }
        
        case ':': /*check for end of sentence*/
        case ';': /*check for end of sentence*/
        case ',': /*check for end of sentence*/          
        {
          data_string[pntr]=data_char;      /*add char to string*/
          pntr++;                           /*increment pointer*/
          if(data_string[0] != '-') /*when the string starts with a - it may very likely contain a config command, in that case we must wait out for the entire string before we process it*/
          {
            data_string[pntr]=0;                  /*add end of string marker*/
            pntr = 0;                             /*reset pointer*/
            data_char = 0;                      /*this will set prev_data_char to a non-space character further in the code*/            
            Process_string(data_string, false);   /*process string but do not add a pause, the comma indicates that more text is comming, a large pause would make it sound awkward*/
          }
          break;
        }
           
        default:
        {
          data_string[pntr]=data_char;  /*add character to string*/
          pntr++;  
          break;
        }
      }
      prev_data_char = data_char;
    }     
  }    
}


/*----------------------------------------------------------------*/
/*                       S U B R O U T I N E S                    */
/*----------------------------------------------------------------*/

void dbg_message(char *msg)
{ 
  switch(settings_struct.debugmode)
  {
    case 1:
    {
      sam->SetSpeed(DEFAULT_SPEED);                 /*set SAM's voice to the standard voice for speaking debug messages, so that you can hear it even if things go wrong*/ 
      sam->SetPitch(DEFAULT_PITCH);                   
      sam->SetThroat(DEFAULT_THROAT);                  
      sam->SetMouth(DEFAULT_MOUTH);  
      sam->SetSingMode(DEFAULT_SINGMODE);    
      sam->SetPhonetic(DEFAULT_PHONETIC);             
    
      sam->Say(out, msg);                           /*output the debug message to the speech synth*/
    
      sam->SetSpeed(settings_struct.spd);                   /*set SAM's voice to the settings as specified by the user*/
      sam->SetPitch(settings_struct.pitch);                   
      sam->SetThroat(settings_struct.throat);                  
      sam->SetMouth(settings_struct.mouth);        
      sam->SetSingMode(settings_struct.singmode);              
      sam->SetPhonetic(settings_struct.phonetic);
      break;
    }

    case 2:
    {      
      Send_BBTXD_string(msg);                       /*output the debug message to bitbanged serial port*/
      break;
    }

    case 0:
    default:
    {
      /*do nothing*/
      break;
    }    
  }
}

/*This routine will process the string*/
/*It will check if it contains a command for settings,*/
/*if not then processes it as speech*/
unsigned char Process_string(char *str, unsigned char add_extra_pause)
{
  int value;                    /*temporary register*/
  unsigned char lp;
  unsigned char i;
  unsigned char stop_loop;
  char phonectic_pronunciation[MAX_STRING_SIZE];  /*the CBM does not fit any more chars then 80 on a single line for the most expensive model*/  
  char single_word[MAX_STRING_SIZE];


  if(str[0] == 0) /*if the string is empty then there is nothing to process*/
  {
    return(false);
  }

  digitalWrite(LED_BUSY, HIGH);                   /*switch it on*/  

  /*To do: this would be a nice place to check for exceptions, so filter out words or sentences*/
  
  if(Check_keyword(str, "-demo") == true)         /*check for keyword: -demo*/  
  {
    Play_demo_1();                                /*play the demo...*/
  }
  else
  if(Check_keyword(str, "-config") == true)       /*check for keyword: -config*/  
  {                                               /*==========================*/
    dbg_message("config ");
    stop_loop = false;
    while((str[0]>0) && stop_loop==false)
    {
      stop_loop = true;  /*stop loop, unless we find something, this makes it possible to keep looping untill nothing more is found*/
      if(Check_keyword(str, " ") == true)                         /*check for possible [space] character*/  
      {                                                           /*====================================*/
        /*there may be more commands in the string seperated by one or more spaces*/  
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "show") == true)                      /*check for keyword: show*/  
      {                                                           /*=======================*/
        dbg_message("show");                                      
        ShowSettings(1);                                          /*show the settings, send the data to the alternative serial port*/
      }      
      else
      if(Check_keyword(str, "debug") == true)                     /*check for keyword: debug*/  
      {                                                           /*========================*/
        dbg_message("debug");
        value = Get_value(str);                                   /*try to read value*/
        if(value >= 0)
        {
          settings_struct.debugmode = value;                      /*save to working register for later use*/
        }
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "speed") == true)                     /*check for keyword: speed*/  
      {                                                           /*========================*/
        dbg_message("speed");
        value = Get_value(str);                                   /*try to read value*/
        if(value >= 0)
        {
          settings_struct.spd = value;                            /*save to working register for later use*/
          sam->SetSpeed(settings_struct.spd);                     /*apply new value*/      
        }
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "pitch") == true)                     /*check for keyword: pitch*/  
      {                                                           /*========================*/
        dbg_message("pitch");
        value = Get_value(str);                                   /*try to read value*/
        if(value >= 0)
        {
          settings_struct.pitch = value;                          /*save to working register for later use*/
          sam->SetPitch(settings_struct.pitch);                   /*apply new value*/      
        }
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "throat") == true)                    /*check for keyword: throat*/  
      {                                                           /*=========================*/
        dbg_message("throat");
        value = Get_value(str);                                   /*try to read value*/
        if(value >= 0)
        {
          settings_struct.throat = value;                         /*save to working register for later use*/
          sam->SetThroat(settings_struct.throat);                 /*apply new value*/      
        }
        stop_loop = false;                                        /*execute the while loop, once more*/        
      }
      else
      if(Check_keyword(str, "mouth") == true)                     /*check for keyword: mouth*/  
      {                                                           /*========================*/
        dbg_message("mouth");
        value = Get_value(str);                                   /*try to read value*/
        if(value >= 0)
        {
          settings_struct.mouth = value;                          /*save to working register for later use*/
          sam->SetMouth(settings_struct.mouth);                   /*apply new value*/      
        }
        stop_loop = false;                                        /*execute the while loop, once more*/        
      }
      else
      if(Check_keyword(str, "singmode") == true)                  /*check for keyword: singmode*/  
      {                                                           /*===========================*/
        dbg_message("sing mode");
        value = Get_value(str);                                   /*try to read value*/
        if(value == 0) {settings_struct.singmode = false;}        /*value 0 = false (off)*/
        else           {settings_struct.singmode = true;}         /*value 1 (or anything else) = true (on)*/
        sam->SetSingMode(settings_struct.singmode);               /*apply new value*/      
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "phonetic") == true)                  /*check for keyword: phonetic*/  
      {                                                           /*===========================*/
        dbg_message("phonetic mode");
        value = Get_value(str);                                   /*try to read value*/
        if(value == 0) {settings_struct.phonetic = false;}        /*value 0 = false (off)*/
        else           {settings_struct.phonetic = true;}         /*value 1 (or anything else) = true (on)*/
        sam->SetPhonetic(settings_struct.phonetic);               /*apply new value*/      
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "pause") == true)                     /*check for keyword: pause*/  
      {                                                           /*========================*/
        dbg_message("pause");
        value = Get_value(str);                                   /*try to read value*/
        settings_struct.pause = value;                            /*save value to register*/
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "msg1") == true)                      /*check for keyword: msg1*/  
      {                                                           /*=======================*/
        Get_string(str, c_string, WELCOME_MSG_SIZE);              /*get the new message string from the string as send by the user*/
        settings_struct.msg1 = c_string;                          /*copy string to final variable destination*/
        dbg_message("has been stored as welcome message 1");      /*show some debug info*/
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "msg2") == true)                      /*check for keyword: msg2*/  
      {                                                           /*=======================*/
        Get_string(str, c_string, WELCOME_MSG_SIZE);              /*get the new message string from the string as send by the user*/
        settings_struct.msg2 = c_string;                          /*copy string to final variable destination*/
        dbg_message("has been stored as welcome message 2");      /*show some debug info*/
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "load") == true)                      /*check for keyword: load*/  
      {                                                           /*=======================*/
        SettingsLoad();                                             /*load settings from non-volatile memory*/
        dbg_message("loading settings");                          /*show some debug info*/
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "save") == true)                      /*check for keyword: save*/  
      {                                                           /*=======================*/
        SettingsSave();                                             /*save the current settings to non-volatile memory*/
        dbg_message("saving settings");                           /*show some debug info*/
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "default") == true)                   /*check for keyword: default*/  
      {                                                           /*==========================*/
        SettingsDefault();                                        /*get the default settings but do not store them, if the user is sure, he/she must confirma this by also giving the save command*/
        dbg_message("restoring settings");                        /*show some debug info*/
        stop_loop = false;                                        /*execute the while loop, once more*/
      }
      else
      if(Check_keyword(str, "update") == true)                    /*check for keyword: update*/  
      {                                                           /*=======================*/
        dbg_message("update");
        StartUpdateServer();                                      /*start the update web server*/
      }      
      else
      if(Check_keyword(str, "filebrowser") == true)               /*check for keyword: filebrowser*/  
      {                                                           /*=======================*/
        dbg_message("filebrowser");
        StartFileBrowser();                                       /*start the update web server*/
      }      
      else      
      if(Check_keyword(str, "ssid") == true)                      /*check for keyword: ssid*/  
      {                                                           /*=======================*/
        Get_string(str, c_string, NETWORK_CFG_SIZE);              /*get the new message string from the string as send by the user*/
        settings_struct.ssid = c_string;                          /*copy string to final variable destination*/
        dbg_message("network name has been received");            /*show some debug info*/  
      }      
      else      
      if(Check_keyword(str, "pass") == true)                      /*check for keyword: pass*/  
      {                                                           /*=======================*/
        Get_string(str, c_string, NETWORK_CFG_SIZE);              /*get the new message string from the string as send by the user*/
        settings_struct.pass = c_string;                          /*copy string to final variable destination*/        
        dbg_message("network password has been received");        /*show some debug info*/
      }
      else      
      if(Check_keyword(str, "telnethost") == true)                /*check for keyword: ...*/  
      {                                                           /*=======================*/
        Get_string(str, c_string, WELCOME_MSG_SIZE);              /*get the new message string from the string as send by the user*/
        settings_struct.tnethost = c_string;                      /*copy string to final variable destination*/        
        dbg_message("telnet hostname has been received");         /*show some debug info*/
      }
      else            
      if(Check_keyword(str, "telnetport") == true)                /*check for keyword: ...*/  
      {                                                           /*=======================*/
        Get_string(str, c_string, WELCOME_MSG_SIZE);              /*get the new message string from the string as send by the user*/
        settings_struct.tnetport = c_string;                      /*copy string to final variable destination*/        
        dbg_message("telnet hostname has been received");         /*show some debug info*/
      }
      else                  
      if(Check_keyword(str, "telnetstart") == true)               /*check for keyword: ...*/  
      {                                                           /*=======================*/        
        dbg_message("telnet request has been received");          /*show some debug info*/
        StartTelnetClient();                                      /*start the telnet client*/
      }
      else
      if(Check_keyword(str, "serialdelay") == true)               /*check for keyword: ...*/  
      {                                                           /*========================*/
        dbg_message("additional delay");
        value = Get_value(str);                                   /*try to read value*/
        if(value >= 0)
        {
          settings_struct.serialdel = value;                      /*save to working register for later use*/
        }
        stop_loop = false;                                        /*execute the while loop, once more (because there might be more info (other commands) on this line)*/
      }            
      else      
      {
        dbg_message("Syntax error, unkonwn command");       /*say syntax error with default voice*/
        //sam->Say(out, "Syntax error, unknown command.");  /*say syntax error with current voice (which might be inappropriate and confusing)*/
      }
    }
  }
  else
  {                           /*The configuration keyword has not been found, so the string is to be considered as purely speech*/
    if(str[0] > 0)            /*prevent pronuncing empty strings*/
    {
      /*When using the dictionary and not in phonetic mode, we must search for each word in the sentence for it's pronunciation in the dictionary*/
      /*If the word is found, speak it accordingly, otherwise speak it usings SAM's speech rules*/
      if((settings_struct.dictionary == true) && (settings_struct.phonetic == false)) /*check if dictionary is enabled*/
      {
        /*slice up the sentence into individual words*/      
        i=0;
        lp=0;
        while(lp<MAX_STRING_SIZE)
        {                    
          if((str[lp] == 0) || (str[lp] == ' ') || (str[lp] == ',') || (str[lp] == '.') || (str[lp] == '!') || (str[lp] == '?'))
          {
            if((str[lp] == '.') || (str[lp] == '!') || (str[lp] == '?'))  /*make sure that we don't fortget to use the ".!?", since these rare included for a reason so they must be used too*/
            {
              single_word[i] = str[lp];
              i++;
            }           
            single_word[i] = 0; /*add end of string indicator*/          
            if(single_word[0] != 0)
            {
              if(Dictionary_search(&primary_dict, single_word, phonectic_pronunciation))  /*search for the word in the primary dictionary*/
              {            
                //Send_BBTXD_string("p:"); /*DEBUG!!!!*/                
                //Send_BBTXD_string(single_word); /*DEBUG!!!!*/
                //Send_BBTXD_string("="); /*DEBUG!!!!*/                
                //Send_BBTXD_string(phonectic_pronunciation); /*DEBUG!!!!*/
                //Send_BBTXD_string("-"); /*DEBUG!!!!*/

                sam->SetPhonetic(true);                     /*set SAM into phonetic mode to use the dictionaries pronunciation*/
                sam->Say(out, phonectic_pronunciation);     /*speak the word according to the info from the dictionary*/
                sam->SetPhonetic(settings_struct.phonetic); /*set back to what it was*/
              }
              else
              if(Dictionary_search(&secondary_dict, single_word, phonectic_pronunciation))  /*search for the word in the secondary dictionary*/
              {            
                //Send_BBTXD_string("s:"); /*DEBUG!!!!*/                                
                //Send_BBTXD_string(single_word); /*DEBUG!!!!*/
                //Send_BBTXD_string("="); /*DEBUG!!!!*/                
                //Send_BBTXD_string(phonectic_pronunciation); /*DEBUG!!!!*/
                //Send_BBTXD_string("-"); /*DEBUG!!!!*/                

                sam->SetPhonetic(true);                     /*set SAM into phonetic mode to use the dictionaries pronunciation*/
                sam->Say(out, phonectic_pronunciation);  /*speak the word according to the info from the dictionary*/
                sam->SetPhonetic(settings_struct.phonetic); /*set back to what it was*/
              }
              else
              {
                sam->Say(out, single_word);  /*speak the word according to the info from the dictionary*/
              }
            }

            if(str[lp]==0)  /*end of stringreached, then exit while-loop*/
            {
              break;
            }
            i=0;
            lp++;
          }
          else
          {
            single_word[i] = str[lp];
            i++;
            lp++;
          }          
        }
        if((add_extra_pause==true) && (settings_struct.pause > 0))
        {
          delay(10*settings_struct.pause);  /*add a small delay after each sentence*/
        }        
      }
      else
      {
        sam->Say(out, str);
        if((add_extra_pause==true) && (settings_struct.pause > 0))
        {
          delay(10*settings_struct.pause);  /*add a small delay after each sentence*/
        }        
      }      
    }
  }

  digitalWrite(LED_BUSY, LOW);                  /*switch it off*/  
  return(true);  
}


/*a simple routine to remove the part of the string (the front part, which is already processed)*/
unsigned char Check_keyword(char *str, char *keyword)
{
  unsigned char cmp_strlen;     /*used for the length of the string (used for readabillity)*/
  
  cmp_strlen = strlen(keyword);
  if (strncmp(str, keyword, cmp_strlen) == 0)
  {
    Remove_processed(str, cmp_strlen);              /*remove the part we've just procesed*/      
    return(true);
  }
  return(false);
}


/*a simple routine to remove the part of the string (the front part, which is already processed)*/
void Remove_processed(char *str, unsigned char len)
{
  unsigned char pntr;
    
  pntr=0;
  while(str[pntr+len] > 0)
  {
    str[pntr] = str[pntr+len];
    pntr++;
  }
  str[pntr] = 0;
}


/*This routine will take a character of the PETSCII kind and makes it a ASCII kind*/
/*THE CODE BELOW COULD BE MUCH MORE COMPACT, HOWEVER, THIS FORM WAS CHOSEN BECAUSE IT OFFERED THE GREATEST OVERVIEW AND THEREFORE ALLOWED FOR THE GREATEST CHANCE OF HAVING THE LEAST AMOUNT OF BUGS*/
char PETSCII_to_ASCII(char c)
{
  char ascii_val;
  
  switch(c)
  {
    case 0x20:  {ascii_val=' '; break;}
    case 0x21:  {ascii_val='!'; break;}
    case 0x22:  {ascii_val='"'; break;}
    case 0x23:  {ascii_val='#'; break;}
    case 0x24:  {ascii_val='$'; break;}
    case 0x25:  {ascii_val='%'; break;}
    case 0x26:  {ascii_val='&'; break;}
    case 0x27:  {ascii_val=39; break;}  /*39='''*/
    case 0x28:  {ascii_val='('; break;}
    case 0x29:  {ascii_val=')'; break;}
    case 0x2A:  {ascii_val='*'; break;}
    case 0x2B:  {ascii_val='+'; break;}
    case 0x2C:  {ascii_val=','; break;}
    case 0x2D:  {ascii_val='-'; break;}
    case 0x2E:  {ascii_val='.'; break;}
    case 0x2F:  {ascii_val='/'; break;}
          
    case 0x30:  {ascii_val='0'; break;}
    case 0x31:  {ascii_val='1'; break;}
    case 0x32:  {ascii_val='2'; break;}
    case 0x33:  {ascii_val='3'; break;}
    case 0x34:  {ascii_val='4'; break;}
    case 0x35:  {ascii_val='5'; break;}
    case 0x36:  {ascii_val='6'; break;}
    case 0x37:  {ascii_val='7'; break;}
    case 0x38:  {ascii_val='8'; break;}
    case 0x39:  {ascii_val='9'; break;}
    case 0x3A:  {ascii_val=':'; break;}
    case 0x3B:  {ascii_val=';'; break;}
    case 0x3C:  {ascii_val='<'; break;}
    case 0x3D:  {ascii_val='='; break;}
    case 0x3E:  {ascii_val='>'; break;}
    case 0x3F:  {ascii_val='?'; break;}

    case 0x40:  {ascii_val='@'; break;}
    case 0x41:  {ascii_val='a'; break;}
    case 0x42:  {ascii_val='b'; break;}
    case 0x43:  {ascii_val='c'; break;}
    case 0x44:  {ascii_val='d'; break;}
    case 0x45:  {ascii_val='e'; break;}
    case 0x46:  {ascii_val='f'; break;}
    case 0x47:  {ascii_val='g'; break;}
    case 0x48:  {ascii_val='h'; break;}
    case 0x49:  {ascii_val='i'; break;}
    case 0x4A:  {ascii_val='j'; break;}
    case 0x4B:  {ascii_val='k'; break;}
    case 0x4C:  {ascii_val='l'; break;}
    case 0x4D:  {ascii_val='m'; break;}
    case 0x4E:  {ascii_val='n'; break;}
    case 0x4F:  {ascii_val='o'; break;}

    case 0x50:  {ascii_val='p'; break;}
    case 0x51:  {ascii_val='q'; break;}
    case 0x52:  {ascii_val='r'; break;}
    case 0x53:  {ascii_val='s'; break;}
    case 0x54:  {ascii_val='t'; break;}
    case 0x55:  {ascii_val='u'; break;}
    case 0x56:  {ascii_val='v'; break;}
    case 0x57:  {ascii_val='w'; break;}
    case 0x58:  {ascii_val='x'; break;}
    case 0x59:  {ascii_val='y'; break;}
    case 0x5A:  {ascii_val='z'; break;}
    case 0x5B:  {ascii_val='['; break;}
  //case 0x5C:  {ascii_val=''; break;}
    case 0x5D:  {ascii_val=']'; break;}
  //case 0x5E:  {ascii_val=''; break;}
  //case 0x5F:  {ascii_val=''; break;}

  //case 0xc0:  {ascii_val=''; break;}
    case 0xc1:  {ascii_val='A'; break;}
    case 0xc2:  {ascii_val='B'; break;}
    case 0xc3:  {ascii_val='C'; break;}
    case 0xc4:  {ascii_val='D'; break;}
    case 0xc5:  {ascii_val='E'; break;}
    case 0xc6:  {ascii_val='F'; break;}
    case 0xc7:  {ascii_val='G'; break;}
    case 0xc8:  {ascii_val='H'; break;}
    case 0xc9:  {ascii_val='I'; break;}
    case 0xcA:  {ascii_val='J'; break;}
    case 0xcB:  {ascii_val='K'; break;}
    case 0xcC:  {ascii_val='L'; break;}
    case 0xcD:  {ascii_val='M'; break;}
    case 0xcE:  {ascii_val='N'; break;}
    case 0xcF:  {ascii_val='O'; break;}

    case 0xd0:  {ascii_val='P'; break;}
    case 0xd1:  {ascii_val='Q'; break;}
    case 0xd2:  {ascii_val='R'; break;}
    case 0xd3:  {ascii_val='S'; break;}
    case 0xd4:  {ascii_val='T'; break;}
    case 0xd5:  {ascii_val='U'; break;}
    case 0xd6:  {ascii_val='V'; break;}
    case 0xd7:  {ascii_val='W'; break;}
    case 0xd8:  {ascii_val='X'; break;}
    case 0xd9:  {ascii_val='Y'; break;}
    case 0xdA:  {ascii_val='Z'; break;}
  //case 0xdB:  {ascii_val=''; break;}
  //case 0xdC:  {ascii_val=''; break;}
  //case 0xdD:  {ascii_val=''; break;}
  //case 0xdE:  {ascii_val=''; break;}
  //case 0xdF:  {ascii_val=''; break;}                 
    
    default: {ascii_val = c; break;}  /*no conversion available*/
  } 

  return(ascii_val);
}

/*This routine will take a character of the ASCII kind and makes it a PETSCII (very important for case sensitive data)*/
/*THE CODE BELOW COULD BE MUCH MORE COMPACT, HOWEVER, THIS FORM WAS CHOSEN BECAUSE IT OFFERED THE GREATEST OVERVIEW AND THEREFORE ALLOWED FOR THE GREATEST CHANCE OF HAVING THE LEAST AMOUNT OF BUGS*/
char ASCII_to_PETSCII(char c)
{
  char petscii_val;
  
  switch(c)
  {
    case ' ':   {petscii_val=0x20; break;}
    case '!':   {petscii_val=0x21; break;}
    case '"':   {petscii_val=0x22; break;}
    case '#':   {petscii_val=0x23; break;}
    case '$':   {petscii_val=0x24; break;}
    case '%':   {petscii_val=0x25; break;}
    case '&':   {petscii_val=0x26; break;}
    case 39:    {petscii_val=0x27; break;}
    case '(':   {petscii_val=0x28; break;}
    case ')':   {petscii_val=0x29; break;}
    case '*':   {petscii_val=0x2A; break;}
    case '+':   {petscii_val=0x2B; break;}
    case ',':   {petscii_val=0x2C; break;}
    case '-':   {petscii_val=0x2D; break;}
    case '.':   {petscii_val=0x2E; break;}
    case '/':   {petscii_val=0x2F; break;}
          
    case '0':   {petscii_val=0x30; break;}
    case '1':   {petscii_val=0x31; break;}
    case '2':   {petscii_val=0x32; break;}
    case '3':   {petscii_val=0x33; break;}
    case '4':   {petscii_val=0x34; break;}
    case '5':   {petscii_val=0x35; break;}
    case '6':   {petscii_val=0x36; break;}
    case '7':   {petscii_val=0x37; break;}
    case '8':   {petscii_val=0x38; break;}
    case '9':   {petscii_val=0x39; break;}
    case ':':   {petscii_val=0x3A; break;}
    case ';':   {petscii_val=0x3B; break;}
    case '<':   {petscii_val=0x3C; break;}
    case '=':   {petscii_val=0x3D; break;}
    case '>':   {petscii_val=0x3E; break;}
    case '?':   {petscii_val=0x3F; break;}

    case '@':   {petscii_val=0x40; break;}
    case 'a':   {petscii_val=0x41; break;}
    case 'b':   {petscii_val=0x42; break;}
    case 'c':   {petscii_val=0x43; break;}
    case 'd':   {petscii_val=0x44; break;}
    case 'e':   {petscii_val=0x45; break;}
    case 'f':   {petscii_val=0x46; break;}
    case 'g':   {petscii_val=0x47; break;}
    case 'h':   {petscii_val=0x48; break;}
    case 'i':   {petscii_val=0x49; break;}
    case 'j':   {petscii_val=0x4A; break;}
    case 'k':   {petscii_val=0x4B; break;}
    case 'l':   {petscii_val=0x4C; break;}
    case 'm':   {petscii_val=0x4D; break;}
    case 'n':   {petscii_val=0x4E; break;}
    case 'o':   {petscii_val=0x4F; break;}

    case 'p':   {petscii_val=0x50; break;}
    case 'q':   {petscii_val=0x51; break;}
    case 'r':   {petscii_val=0x52; break;}
    case 's':   {petscii_val=0x53; break;}
    case 't':   {petscii_val=0x54; break;}
    case 'u':   {petscii_val=0x55; break;}
    case 'v':   {petscii_val=0x56; break;}
    case 'w':   {petscii_val=0x57; break;}
    case 'x':   {petscii_val=0x58; break;}
    case 'y':   {petscii_val=0x59; break;}
    case 'z':   {petscii_val=0x5A; break;}
    case '[':   {petscii_val=0x5B; break;}
  //case '':    {petscii_val=0x5C; break;}
    case ']':   {petscii_val=0x5D; break;}
  //case '':    {petscii_val=0x5E; break;}
  //case '':    {petscii_val=0x5F; break;}

  //case '':    {petscii_val=0xC0; break;}
    case 'A':   {petscii_val=0xC1; break;}
    case 'B':   {petscii_val=0xC2; break;}
    case 'C':   {petscii_val=0xC3; break;}
    case 'D':   {petscii_val=0xC4; break;}
    case 'E':   {petscii_val=0xC5; break;}
    case 'F':   {petscii_val=0xC6; break;}
    case 'G':   {petscii_val=0xC7; break;}
    case 'H':   {petscii_val=0xC8; break;}
    case 'I':   {petscii_val=0xC9; break;}
    case 'J':   {petscii_val=0xCA; break;}
    case 'K':   {petscii_val=0xCB; break;}
    case 'L':   {petscii_val=0xCC; break;}
    case 'M':   {petscii_val=0xCD; break;}
    case 'N':   {petscii_val=0xCE; break;}
    case 'O':   {petscii_val=0xCF; break;}

    case 'P':   {petscii_val=0xD0; break;}
    case 'Q':   {petscii_val=0xD1; break;}
    case 'R':   {petscii_val=0xD2; break;}
    case 'S':   {petscii_val=0xD3; break;}
    case 'T':   {petscii_val=0xD4; break;}
    case 'U':   {petscii_val=0xD5; break;}
    case 'V':   {petscii_val=0xD6; break;}
    case 'W':   {petscii_val=0xD7; break;}
    case 'X':   {petscii_val=0xD8; break;}
    case 'Y':   {petscii_val=0xD9; break;}
    case 'Z':   {petscii_val=0xDA; break;}
  //case '':    {petscii_val=0xDB; break;}
  //case '':    {petscii_val=0xDC; break;}
  //case '':    {petscii_val=0xDD; break;}
  //case '':    {petscii_val=0xDE; break;}
  //case '':    {petscii_val=0xDF; break;}                 
    
    default: {petscii_val = c; break;}  /*no conversion available*/
  } 

  return(petscii_val);
}

/*this routine will get a value from a string*/
/*the value must be within the range 0-255, when outside this range*/
/*or when no value can be found, the routine return the value -1*/
int Get_value(char *str)
{
  char digits = 0;
  int value = -1;   /*-1=invalid value*/
  char msg[16];  
  unsigned char stop_loop = false;

  /*remove any trailing spaces*/
  while(str[0] == ' ')
  {
    Remove_processed(str, 1); /*remove the character ([space]) we've just procesed*/ 
  }

  /*scan value*/
  stop_loop = false;
  while(stop_loop == false)
  {
    if((str[0]>='0') && (str[0]<='9'))
    {
      if(digits++ == 0) {value = 0;}          /*erase the -1 as we seem to have discoverd a valid digit*/
      else              {value = value*10;}   /*every next digit requires value to be multiplied by 10*/
  
      value = value + (str[0] - '0');     /*get the numerical value of the character*/
      Remove_processed(str, 1);           /*remove the part we've just procesed*/     
    }
    else
    {
      stop_loop = true;
    }

    /*check if value exceeds the limit, if so then discard it*/
    if((value <0) || (value > 255))
    {
      value = -1;
      dbg_message("is invalid");      
      return(value);                    /*exit the loop immediately*/
    }
  }

  /*if we end up here, then the value was valid*/
  sprintf(msg, "= %d.",value);
  dbg_message(msg);
  return(value);
}

/*this routine will get a string from a string*/
/*it removes any trailing spaces*/
void Get_string(char *str, char *result, unsigned char maxsize)
{
  char msg[160];  
  unsigned char stop_loop = false;
  unsigned char lp;

  /*remove any trailing spaces*/
  while(str[0] == ' ')
  {
    Remove_processed(str, 1); /*remove the character ([space]) we've just procesed*/ 
  }

  /*scan string*/ 
  lp=0;                                   /*copy text given by user to the string array*/
  while((str[0] != 0) && (lp<maxsize))
  {
    result[lp] = str[0];    
    Remove_processed(str, 1);             /*remove the character ([space]) we've just procesed*/ 
    lp++;
  }            
  result[lp] = 0;                         /*add termination character*/       

  /*if we end up here, then the value was valid*/
  sprintf(msg, "%s",result);
  dbg_message(msg);
}


/*due to the problems of serial ports pins mixed with the I2S data output pins*/
/*another serial port was required, the only option was a software bitbanged serial port*/
/*a bit banged serial function, crude but effective*/
void Send_BBTXD_byte(char data)
{
  #define del_val 1000000/CBM_BAUDRATE  /*calculate delay value in microseconds*/
  //#define del_val 416  /*2400baud equals 416us per bit)*/
  unsigned char i;
  int lp;


  data = ASCII_to_PETSCII(data);  /*convert characters into a format that the CBM can read*/

  yield();          /*handle the things that need to be handle before we starve the system with our long delays*/

  /*-------------------------------------------------------------------------------------*/
  ets_intr_lock();    /*prevent all sorts of interrupts that might screw up the critical timing below*/
  /*.....................................................................................*/

#define FASTBBTXD  1 << BB_TXD
  
  i=8;                            /*number of bits in a byte*/
  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, FASTBBTXD);    /*set IO to 0 (generate startbit)*/
  //delayMicroseconds(del_val);     /*wait untill the start-bit time has passed*/
  /*for some strange reason the C64 is much more reliable when the start bit take a littlebit longer*/
  delayMicroseconds(500);         /*to long startbit! (should be 416us, however this gives more reliable results on my PAL C64)*/  
  while(i>0)                      /*send 8 serial bits, LSB first*/
  {
    if((data & 0x01) == 0x01)     /*check if bit is set or cleared*/
    {
      GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, FASTBBTXD);    /*set IO to 1*/
    }
    else
    {
      GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, FASTBBTXD);    /*set IO to 0*/
    }

    data = data >> 1;             /*rotate right to get next bit*/
    i--;
    delayMicroseconds(del_val);    /*wait until it's time for the next bit*/
  }

  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, FASTBBTXD);    /*set IO to 1 (generate stop-bit)*/
  delayMicroseconds(del_val);     /*wait until the stop-bit time has completely passed (stopbit width)*/
  delayMicroseconds(del_val);     /*for timing-safety reasons (although rs232 specs don't say it is required, but it makes me feel better and it is already slow anyway), we use 2 stop-bits. This gives the CBM a tiny bit more time to get ready for the next byte)*/    

  /*.....................................................................................*/
  ets_intr_unlock();  /*allow interrupts to happen*/
  /*-------------------------------------------------------------------------------------*/  

  /*to give the CBM time to process the data, we MUST wait for a loooooooooooooooong time*/
  /*that time is expressed in characters (the time it takes to send a byte over the serial port)*/
  /*experiments have shown that a delay of "5 bytes" should be sufficient on a c64*/
  lp = settings_struct.serialdel.toInt();
  while(lp>0)
  {
    delayMicroseconds(10*del_val);  /*this is the time of a complete byte*/
    lp--;
  }

}

/*send a string of bytes/characters to the bit-banged-serial-port*/
void Send_BBTXD_string(char *str)
{
  unsigned char pntr;

  pntr=0;
  while(str[pntr]>0)  /*keep sending until end-of-string marker is reached*/
  {  
    Send_BBTXD_byte(str[pntr]);
    pntr++;
  }
}

void ShowSettings(unsigned char destination)
{
  char t_str[200];  /*string used for ptinting text*/
  
  switch(destination)
  {
    case 1:
    {
      sprintf(t_str,"\r");                                      /*make sure we start on an new line*/
      Send_BBTXD_string(t_str);                                 /*output the text to he bitbanged serial port*/      
      delay(1000);                                              /*long delay so that we make sure that we don't loose any data, as we cannot be sure that the CBM handles everything fast enough*/

      sprintf(t_str,"firmware =" __DATE__ "\r");                    Send_BBTXD_string(t_str); delay(250);                 
      sprintf(t_str,"speed    =%d\r",settings_struct.spd);          Send_BBTXD_string(t_str); delay(250);           
      sprintf(t_str,"pitch    =%d\r",settings_struct.pitch);        Send_BBTXD_string(t_str); delay(250);
      sprintf(t_str,"throat   =%d\r",settings_struct.throat);       Send_BBTXD_string(t_str); delay(250);
      sprintf(t_str,"mouth    =%d\r",settings_struct.mouth);        Send_BBTXD_string(t_str); delay(250);
      sprintf(t_str,"singmode =%d\r",settings_struct.singmode);     Send_BBTXD_string(t_str); delay(250);
      sprintf(t_str,"phonetic =%d\r",settings_struct.phonetic);     Send_BBTXD_string(t_str); delay(250);
      sprintf(t_str,"pause    =%d\r",settings_struct.pause);        Send_BBTXD_string(t_str); delay(250);
      sprintf(t_str,"debugmode=%d\r",settings_struct.debugmode);    Send_BBTXD_string(t_str); delay(250);
      settings_struct.msg1.toCharArray(c_string, (settings_struct.msg1.length()+1));  /*our variable is stored as a string, but we need a char array*/    
      sprintf(t_str,"msg1     =%s\r",c_string);                     Send_BBTXD_string(t_str); delay(250);
      settings_struct.msg2.toCharArray(c_string, (settings_struct.msg2.length()+1));  /*our variable is stored as a string, but we need a char array*/          
      sprintf(t_str,"msg2     =%s\r",c_string);                     Send_BBTXD_string(t_str); delay(250);

      /*the lines/variables below are not printed and therefore invisble, because they are not used but may be of use in the future*/
      settings_struct.ssid.toCharArray(c_string, (settings_struct.ssid.length()+1));  /*our variable is stored as a string, but we need a char array*/          
      sprintf(t_str,"ssid     =%s\r",c_string);                     Send_BBTXD_string(t_str); delay(500);
      settings_struct.pass.toCharArray(c_string, (settings_struct.pass.length()+1));  /*our variable is stored as a string, but we need a char array*/          
      sprintf(t_str,"pass     =%s\r",c_string);                     Send_BBTXD_string(t_str); delay(500);     
      break;
    }

    case 0:
    default:
    {
      Serial.println(F("Firmware version:" __DATE__ ));
      
      Serial.printf("speed     = %d\n",settings_struct.spd);
      Serial.printf("pitch     = %d\n",settings_struct.pitch);
      Serial.printf("throat    = %d\n",settings_struct.throat);
      Serial.printf("mouth     = %d\n",settings_struct.mouth);
      Serial.printf("singmode  = %d\n",settings_struct.singmode);
      Serial.printf("phonetic  = %d\n",settings_struct.phonetic);
      Serial.printf("pause     = %d\n",settings_struct.pause);
      Serial.printf("debugmode = %d\n",settings_struct.debugmode);
      settings_struct.msg1.toCharArray(c_string, (settings_struct.msg1.length()+1));  /*our variable is stored as a string, but we need a char array*/                      
      Serial.printf("msg1      = %s\n",c_string);
      settings_struct.msg2.toCharArray(c_string, (settings_struct.msg2.length()+1));  /*our variable is stored as a string, but we need a char array*/                      
      Serial.printf("msg2      = %s\n",c_string);

      settings_struct.ssid.toCharArray(c_string, (settings_struct.ssid.length()+1));  /*our variable is stored as a string, but we need a char array*/                
      Serial.printf("SSID      = %s\n",c_string);
      settings_struct.pass.toCharArray(c_string, (settings_struct.pass.length()+1));  /*our variable is stored as a string, but we need a char array*/                
      Serial.printf("PASSWORD  = %s\n",c_string);
      break;
    }    
  }
 
}


/*this routine connect this device to the specified wifi network*/
void ConnectToWifi(void)
{
  char t_str[200];  /*string used for printing text*/

  WiFi.disconnect();                            /*only required when the ESP8266 is reset, this is not required when it has just been powered-on*/
  
  if(WiFi.status() != WL_CONNECTED)  /*start wifi functionality (if this is not started yet) to allow for a telnet client to be setup*/
  {
    //WiFi.disconnect();                            /*only required when the ESP8266 is reset, this is not required when it has just been powered-on*/
    WiFi.persistent(false);                       /*Do not memorise new wifi connections*/
    WiFi.hostname("SerialSpeechSynthesizerSAM");  /*the name of this device. This name is shown in the list of connected devices in your router*/
    WiFi.mode(WIFI_STA);                          /*connect to an existing WiFi network (the ESP8266 will be used as a STATION)*/
  
    settings_struct.ssid.toCharArray(c_string, (settings_struct.ssid.length()+1));  /*our variable is stored as a string, but we need a char array*/      
    settings_struct.pass.toCharArray(c2_string, (settings_struct.pass.length()+1));  /*our variable is stored as a string, but we need a char array*/      
    WiFi.begin(c_string, c2_string); /*connect to wifi network*/
    sprintf(t_str,"trying to connect to\r");          Send_BBTXD_string(t_str); delay(250);          
    sprintf(t_str,"%s\r",c_string);                   Send_BBTXD_string(t_str); delay(250);
    sprintf(t_str,"%s\r",c2_string);                  Send_BBTXD_string(t_str); delay(250);         
  
    sam->Say(out,"trying to connect to wifi network");
    while (WiFi.status() != WL_CONNECTED)
    {
      sam->Say(out,"please wait");    
      sprintf(t_str,"please wait...\r");              Send_BBTXD_string(t_str); delay(1000);           
    }
  }
}

void StartUpdateServer()
{
  char t_str[200];  /*string used for printing text*/

  ConnectToWifi();    /*connect to specified wifi network*/      
  sam->Say(out,"connected. starting update server.");
  sprintf(t_str,"connected\r");                     Send_BBTXD_string(t_str); delay(250);       
  sprintf(t_str,"starting update server...\r\r");   Send_BBTXD_string(t_str); delay(250);       
  
  httpUpdater.setup(&httpServer);   /*this is the server for the update process*/
  httpServer.begin();               /*let's start it here*/


  /*keep in mind that all text send to the CBM computer must be in upper case in order to print it to screen*/            
  sprintf(t_str,"in order to update,\r");                                                                                       Send_BBTXD_string(t_str); delay(500);       
  sprintf(t_str,"open a webbrowser\r");                                                                                         Send_BBTXD_string(t_str); delay(500);       
  sprintf(t_str,"and go to:\r");                                                                                                Send_BBTXD_string(t_str); delay(500);       
  sprintf(t_str,"http://%d.%d.%d.%d/update\r", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );    Send_BBTXD_string(t_str); delay(500);       
  sam->Say(out,"open your web browser. Then go to: h t t p : / /");
  sprintf(t_str,"%d", WiFi.localIP()[0]);
  sam->Say(out, t_str);  
  sam->Say(out,"dot");  
  sprintf(t_str,"%d", WiFi.localIP()[1]);
  sam->Say(out, t_str);  
  sam->Say(out,"dot");    
  sprintf(t_str,"%d", WiFi.localIP()[2]);
  sam->Say(out, t_str);  
  sam->Say(out,"dot");
  sprintf(t_str,"%d", WiFi.localIP()[3]);
  sam->Say(out, t_str);  
  sam->Say(out,"/ update.");    
  while(1)
  {
    delay(1); /*pet the watchdog*/
    httpServer.handleClient();  /*update/handle the webserver to allow user to upload new firmware into the device*/
  }
}

void StartFileBrowser()
{
  char t_str[200];  /*string used for printing text*/

  ConnectToWifi();    /*connect to specified wifi network*/      
  sam->Say(out,"connected. starting file browser server.");
  sprintf(t_str,"connected\r");                     Send_BBTXD_string(t_str); delay(250);       
  sprintf(t_str,"starting file browser server...\r\r");   Send_BBTXD_string(t_str); delay(250);       
  
  FSBrowser_init();
  
  /*keep in mind that all text send to the CBM computer must be in upper case in order to print it to screen*/            
  sprintf(t_str,"in order to browse,\r");                                                                                     Send_BBTXD_string(t_str); delay(500);       
  sprintf(t_str,"open a webbrowser\r");                                                                                       Send_BBTXD_string(t_str); delay(500);       
  sprintf(t_str,"and go to:\r");                                                                                              Send_BBTXD_string(t_str); delay(500);       
  sprintf(t_str,"http://%d.%d.%d.%d/edit\r", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );    Send_BBTXD_string(t_str); delay(500);       
  sam->Say(out,"open your web browser. Then go to: h t t p : / /");
  sprintf(t_str,"%d", WiFi.localIP()[0]);
  sam->Say(out, t_str);  
  sam->Say(out,"dot");  
  sprintf(t_str,"%d", WiFi.localIP()[1]);
  sam->Say(out, t_str);  
  sam->Say(out,"dot");    
  sprintf(t_str,"%d", WiFi.localIP()[2]);
  sam->Say(out, t_str);  
  sam->Say(out,"dot");
  sprintf(t_str,"%d", WiFi.localIP()[3]);
  sam->Say(out, t_str);  
  sam->Say(out,"/ edit.");    
  while(1)
  {
    delay(1); /*pet the watchdog*/
    FSBrowser_handle();
  }
}

/*                                                  */
/* <<<<<<<<<<< UNDER CONSTRUCTION >>>>>>>>>>>>>>>>> */
/*                                                  */
/*the current status is that it doesn't work they way it should, simply because we cannot push the data fast enough to the CBM without loosing data*/
/*apparently the telnet client uses no handshaking when it's buffers are full, or somethinglike that... anyway. this needs additional research before any*/
/*user is able to use it for decent BBS browsing*/
/*tested it all using: telnet_server = "borderlinebbs.dyndns.org", telnet_port = 6400*/
void StartTelnetClient(void)
{
  unsigned int lp, lp2;
//  unsigned char ut_str[255];  /*string used for printing text*/
  char t_str[255];            /*string used for printing text*/
  #define TELNETDATABUFFERSIZE 1024
  //unsigned char data[TELNETDATABUFFERSIZE];  /*array for the data*/
  char data[TELNETDATABUFFERSIZE];  /*array for the data*/  

  WiFiClient client;

  ConnectToWifi();    /*connect to specified wifi network*/
  sam->Say(out,"connected, starting telnetclient.");
  sprintf(t_str,"connected, starting client\r");        Send_BBTXD_string(t_str); delay(250);     
  
  settings_struct.tnethost.toCharArray(c_string, (settings_struct.tnethost.length()+1));   /*our variable is stored as a string, but we need a char array*/      
  settings_struct.tnetport.toCharArray(c2_string, (settings_struct.tnetport.length()+1));  /*our variable is stored as a string, but we need a char array*/      

  sprintf(t_str,"trying to connect to telnet host:\r"); Send_BBTXD_string(t_str); delay(250);            
  sprintf(t_str,"%s\r",c_string);                       Send_BBTXD_string(t_str); delay(250);
  sprintf(t_str,"at port %s\r",c2_string);              Send_BBTXD_string(t_str); delay(250);           

  sam->Say(out,"trying to connect to telnet host.");
  if (client.connect(c_string, settings_struct.tnetport.toInt()))
  //if (client.connect(telnet_server, telnet_port))
  {    
    sprintf(t_str,"telnet server allows connection\r");        Send_BBTXD_string(t_str); delay(250);     

    while(1)
    {
      yield();  /*pet the watchdog, do important OS related stuff...bladieblayadada*/
      
      /*TELNET_C_SEND:      send data from through this client to the server*/
      /*--------------------------------------------------------------------*/
      if(Serial.available() > 0)
      { 
      //data = PETSCII_to_ASCII(Serial.read()); /*convert the PETSCII chars of the CBM into ASCII values of the same meaning*/
        data[0] = Serial.read(); /*convert the PETSCII chars of the CBM into ASCII values of the same meaning*/        
        client.write(data, 1); /*send the pointer to the buffer holding the data, also send the payloadsize of the buffer*/      
      }
      
      /*TELNET_C_RECEIVE:   get data from the server through this client*/
      /*--------------------------------------------------------------------*/  
      lp=0;
      while((client.available() > 0) && (lp < TELNETDATABUFFERSIZE))
      {
        data[lp] = client.read();                  
        lp++;         
      }

      /*dump the data we've just read to the CBM*/
      lp2=0;
      while(lp>0)
      {
        Send_BBTXD_byte(data[lp2]);
        lp2++;
        lp--;
      }
      
    }
  }

  // possible future functionality:
  // ------------------------------
  // client.stop();  /*disconnect / close client*/
  // client_stat = client.connected();   /*1 = SUCCESS, -1 = TIMED_OUT, -2 = INVALID_SERVER, -3 = TRUNCATED, -4 = INVALID_RESPONSE */

}


/*search for a dictionary and if found, make and index of the starting points of each of the letters of the alphabet*/
/*this we we know exactly where all the words begiing with the letter a are, and the letter b, etc*/
/*it is important that the dictionary is in complete alphabetical order. Just like a normal dictionary (you know... the book)*/
/*Books are things that were invented hundreds of years before the internet. Books contained textual information and sometimes*/
/*pictures. Books were used in schools and entertained people during their time of. There was also a book that described all*/
/*existing words in a language and explained how it should be spoken and what it meant. Those kind of books are called dictionaries*/
/*in order to make it easier for the reader of the book to find a word all words in the book swere at alphabetical order, this saved*/
/*the reader a lot of time and allowed the reader to know very quickly whether a word wasn't in the dictionary*/
/*This technology/knowledge is almost completely forgotten since the introduction of the search engine in the early 21st century.*/
/*This routine returns false when the dictionary file could not be found*/
/*When the dictionary does not contain words with a certain letter of the alphabet, the index is set to 0*/
unsigned char Dictionary_index(dictionary_structTYPE *d)
{
  unsigned char lp;
  unsigned char y=0;  /*a yield counter, if we've read too many lines we must do a yield, to make sure the watchdog doesn't trigger*/  
  unsigned long cur_pos;
  unsigned long pos_of_last_found_letter;
  String line;

  Serial.print("Indexing dictionary: ");
  Serial.println(d->filename);
    
  yield();
  //SPIFFS.begin();
  pos_of_last_found_letter = 0;  
  file = SPIFFS.open(d->filename, "r");    /*this opens the file "f.txt" in read-mode*/  
  if (!file)
  {
    Serial.println("Dictionary file could not be found");
    d->enabled = false; /*indexing went wrong, so the dictionary is disabled as it is not ready for use*/    
    return(false);    
  }
  else
  {  
    for(lp=0; lp<26; lp++)  /* 0='a', 1='b', etc.*/
    { 
      yield();      
      d->index[lp]=0;                             /*this is the default value if we do not find the letter we search for*/      
      file.seek(pos_of_last_found_letter, SeekSet);   /*SeekSet position is set to the position of the last found letter*/
      while(file.available())   /*Lets read line by line from the file*/
      { 
        if(y++==0)  /*make sure we yield every 256 line*/
        {
            yield();  /*handle the things that need to be handle before we starve the system with our long delays*/
        }

        cur_pos = file.position();  /*this is the current location in the file, save it to the index table*/
        line = file.readStringUntil('\n');
        if(line[0] == 'a'+lp) /*check if the word on this line start with the character we are looking for*/
        {       
          d->index[lp] = cur_pos;           /*use the pointer from the position BEFORE the readline, because that is where we want to start the search*/
          pos_of_last_found_letter = cur_pos;   /*also remember this value as the last succesful find*/
          //Serial.println(line);
          //Serial.println(d->index[lp]);   /*this is the current location in the file*/        
          break;                            /*we found what we searched for so exit the while to start a search for the next letter we need to index*/
        }

        if(line[0] > ('a'+ lp)) /*if we discover a letter that is greater than the letter we are looking for, we must conclude that it isn't in the dictionary*/
        {
          //Serial.print("letter ... not found");
          break;          
        }
      }
    }
    Serial.println("Dictionary indexing complete");
  }
  file.close();
  d->enabled = true; /*indexing went OK, so the dictionary can be enabled and is ready for use*/
  return(true);
}

/*this dictionary search RELIES HEAVILY on the fact that dictionaries are in alphabetical order*/
/*MAKE SURE THAT THE DICTIONARY INDEED IS IN ALPHABETICAL ORDER otherwise a word might not be found*/
/*Relying on the alphabetical order allows us to stop searching for a word as quickly as possible, which increases efficiency*/
unsigned char Dictionary_search(dictionary_structTYPE *d, char *searchword, char *phoneticword)
{
  unsigned char i;  /*this is the index for the main lookup table for the first letter*/
  unsigned char lp; /*this is for the letter comparison loop*/
  unsigned char c;  /*here we store the . / ! / ? if found in the single word*/
  unsigned char prev_matchedchars;  /*this is to count the number of matching chars, as soon as this value decreases in comparison to the previous search we know that we passed the point where the word SHOULD have been located if it was defined in the library*/
  String line;
  unsigned char y=0;  /*a yield counter, if we've read too many lines we must do a yield, to make sure the watchdog doesn't trigger*/

  if(d->enabled == false)
  {
    return(false);    /*oops, this dictionary is not enabled, exit immediately*/
  }

  yield();
  phoneticword[0]=0;  /*the string is empty*/
  //SPIFFS.begin();  
  file = SPIFFS.open(d->filename, "r");    /*this opens the file "f.txt" in read-mode*/  
  if (!file)
  {
    /*unexpected error situation*/
  }
  else
  {
    /*to speed up the search we set the file pointer to the location (of which the first letter of the word we are looking) for is located*/
    i= searchword[0] - 'a';

    if(d->index[i]==0)        /*when the index is 0, then the letter isn't in the dictionary and there is no reason to continue*/
    {
      //Serial.print("no words with the letter '");
      //Serial.print(searchword[0]);
      //Serial.println("' in the dictionary");
      file.close();  
      return(false);
    }
    
    file.seek(d->index[i], SeekSet);  /*SeekSet position is set to offset bytes from the start, in other words, the absolute position in the file*/

    prev_matchedchars=0;  /*this counter holds the number of matched characters in the word of the previously searched line in the dictionary*/
    while(file.available())  /*Lets read line by line from the file*/
    {
      if(y++==0)  /*make sure we yield every 256 line*/
      {
          yield();  /*handle the things that need to be handle before we starve the system with our long delays*/
      }
      
      line = file.readStringUntil('\n');
      //Serial.print("\n");             /*for debugging only*/
      for(lp=0; lp<80; lp++) /*we allow searching for a word of up to 80 chars long*/    
      {
        //Serial.print(line[lp]);        /*for debugging only*/
        //Serial.print(",");             /*for debugging only*/
        if(searchword[lp] != line[lp])
        {          
          /*If the first letter doesn't match the first letter of the word from the dictionary we must stop searching completely*/
          /*Because the dictionary is in alphabetically order we KNOW that we've passed all the words starting with that letter*/                 
          if(lp==0)
          {
            //Serial.print("The word ");
            //Serial.print(searchword);
            //Serial.println(" isn't in the dictionary");
            file.close();              
            return(false);
          }

          /*check if we have reached the end of the word and have a perfect match*/          
          c = searchword[lp];
          if(((c == 0) || (c == '.') || (c == '!') || (c == '?')) && (line[lp] == '='))
          {
            /*we found the word we were searching for*/
            //Serial.print("The word ");
            //Serial.print(searchword);
            //Serial.println(" has been found");

            /*copy the phonetic word to the out string*/
            lp++;
            i=0;
            while((line[lp] != 0)  && (line[lp] != '\r') && (line[lp] != '\n') && (line[lp] != ' ') && (i<(MAX_STRING_SIZE-1))) /*anything that could be at the end of the line should make this loop stop, also abort on "space" as these might be accidentally put into the file (at the end of the line where you do not see it*/
            {
              phoneticword[i++]=line[lp++];  /*the string is empty*/
            }

            /*for some reason we can't add a ! because then the word is not spoken, but since it sounds exactly the same as a period, we substitue it for that*/
            if(c=='!')
            {
              c='.';
            }

            phoneticword[i++]=c;  /*end of string indicator or . / ! / ? character*/           
            phoneticword[i]=0;    /*end of string indicator, which is only really required when the previous was . / ! / ? But it is easier to add and extra termination char, then to check if we should add it after an . / ! / ? */
            file.close();         
            return(true);            
          }
          else  /*if the match wasn't complete, the we must consider this a mismatch and must carry on by reading the next line from the dictionary*/
          {
            if(lp < prev_matchedchars)
            {
              /*we found the word we were searching for*/
              //Serial.print("max searchpoint surpassed, ");
              //Serial.print(searchword);
              //Serial.println(" not found");
              file.close();                
              return(false);                          
            }
            else
            {
              prev_matchedchars = lp; /*this search was better than the previous search, save value and carry on*/
              break;
            }
          }
        }
      }      
    }
  }
  file.close();  
  return(false);
}


/*A nice example of the tune (and how to code it) is found here https://www.youtube.com/watch?v=-23mWuagdyw */
/*it shows a video of how to play the recorder (flute), slow and easy and with the notes shown*/
/*Knowingthe notes it is a matter of definingthe notes to pitch values*/
/*Then with those definition the score can be ritten down in a musical (understandable fashion) as reading numbers is just too confusing*/
/*It is a bit of work to get the phoneme's done, but very doable once the notes are already correct*/
/*the latter is a bit of a problem if you aren't musical talented, but the recorder video did the trick*/
/*The notes are defined here: https://www.intmath.com/trigonometric-graphs/music.php */
/*so making the table with the corresponding pitch value is a littlebit of experimenting, but doable*/
void Play_demo_1(void)
{
  sam->SetSpeed(DEFAULT_SPEED);                 /*set SAM's voice to the standard voice for speaking debug messages, so that you can hear it even if things go wrong*/ 
  sam->SetPitch(DEFAULT_PITCH);                   
  sam->SetThroat(DEFAULT_THROAT);                  
  sam->SetMouth(DEFAULT_MOUTH);  
  sam->SetSingMode(DEFAULT_SINGMODE);    
  sam->SetPhonetic(false);             
    
  sam->Say(out, "SAM will now sing for you, the famous melody,");         
  sam->Say(out, "buy cycle build for two. A song also known as: daisy."); /*"buy cycle" sound much better then "bicycle"*/

  delay(1000);

  sam->SetSpeed(90); 
  sam->SetSingMode(true);      /*very important for singing, as it will disable unwanted tone bending*/
  sam->SetPhonetic(true);     /*required to speak phonemes*/       

    /*daisy, daisy, give me your answer do*/
    sam->SetPitch(N2D);  sam->Say(out,"DEYYYYYYYYY");                /*dai*/
    sam->SetPitch(N1B);  sam->Say(out,"ZIYIYIYIYIYIYIY");            /*sy*/
    sam->SetPitch(N1G);  sam->Say(out,"DEYYYYYYYYY");                /*dai*/
    sam->SetPitch(N1D);  sam->Say(out,"ZIYIYIYIYIYIYIY");            /*sy*/
    sam->SetPitch(N1E);  sam->Say(out,"GIXV");                       /*give*/
    sam->SetPitch(N1F);  sam->Say(out,"MIYIY");                      /*me*/
    sam->SetPitch(N1G);  sam->Say(out,"YAOW");                       /*your*/
    sam->SetPitch(N1E);  sam->Say(out,"AEAEAEN");                    /*an*/
    sam->SetPitch(N1G);  sam->Say(out,"SERER");                      /*ser*/
    sam->SetPitch(N1D);  sam->Say(out,"DUXUXUXUXUXUXUXUXUXUXUXUX");  /*do*/
    delay(250);

    /*I'm halve crazy all for the love of you*/
    sam->SetPitch(N1A);  sam->Say(out,"AYYYYYYMM"); 
    sam->SetPitch(N2D);  sam->Say(out,"/HAEAEAEAEAEAEF"); 
    sam->SetPitch(N1B);  sam->Say(out,"KREYYYYYYY"); 
    sam->SetPitch(N1G);  sam->Say(out,"ZIYIYIYIYIYIYIY"); 
    sam->SetPitch(N1E);  sam->Say(out,"AXLL"); 
    sam->SetPitch(N1F_); sam->Say(out,"FAOR"); 
    sam->SetPitch(N1G);  sam->Say(out,"DHER"); 
    sam->SetPitch(N1A);  sam->Say(out,"LUHUHUHV"); 
    sam->SetPitch(N1B);  sam->Say(out,"AXAXV"); 
    sam->SetPitch(N1A);  sam->Say(out,"YUXUXUXUXUXUXUXUXUXUX");
    delay(250);

    /*it won't be a stylish marriage I can't afford a carriage*/
    sam->SetPitch(N1B);  sam->Say(out,"IHT"); 
    sam->SetPitch(N2C);  sam->Say(out,"WOWNT"); 
    sam->SetPitch(N1B);  sam->Say(out,"BIY"); 
    sam->SetPitch(N1A);  sam->Say(out,"ER"); 
    sam->SetPitch(N2D);  sam->Say(out,"STAYYYYY"); 
    sam->SetPitch(N1B);  sam->Say(out,"LIHSH"); 
    sam->SetPitch(N1A);  sam->Say(out,"MAE"); 
    sam->SetPitch(N1G);  sam->Say(out,"RIXIXIXIXIXIXIXIXIXIXIXIXIXJ");
    sam->SetPitch(N1A);  sam->Say(out,"AYY");
    sam->SetPitch(N1B);  sam->Say(out,"KAEAEAEAENT");
    sam->SetPitch(N1G);  sam->Say(out,"ER");
    sam->SetPitch(N1E);  sam->Say(out,"FAOAOAORD");
    sam->SetPitch(N1G);  sam->Say(out,"ER");
    sam->SetPitch(N1E);  sam->Say(out,"KAA");
    sam->SetPitch(N1D);  sam->Say(out,"RIXIXIXIXIXIXIXIXIXIXIXIXIXJ");
    delay(250);

    /*but you look sweet upon the seat of a bicycle made for two*/
    sam->SetPitch(N1D);  sam->Say(out,"BUHT");
    sam->SetPitch(N1G);  sam->Say(out,"YUXUXL");
    sam->SetPitch(N1B);  sam->Say(out,"LUXK");
    sam->SetPitch(N1A);  sam->Say(out,"SWIYIYIYIYT");
    sam->SetPitch(N1D);  sam->Say(out,"ER");
    sam->SetPitch(N1G);  sam->Say(out,"PAAAAAAN");
    sam->SetPitch(N1B);  sam->Say(out,"ER");
    sam->SetPitch(N1A);  sam->Say(out,"SIYIYIYT");
    sam->SetPitch(N1B);  sam->Say(out,"UHV");
    sam->SetPitch(N2C);  sam->Say(out,"ER");
    sam->SetPitch(N2D);  sam->Say(out,"BAY");
    sam->SetPitch(N1B);  sam->Say(out,"SIH");
    sam->SetPitch(N1G);  sam->Say(out,"KUXL");
    sam->SetPitch(N1A);  sam->Say(out,"MEYYYYD");
    sam->SetPitch(N1D);  sam->Say(out,"FER");
    sam->SetPitch(N1G);  sam->Say(out,"TUXUXUXUXUXUXUXUXUXUXUX");
  

  sam->SetSpeed(settings_struct.spd);                   /*restore the users settings*/
  sam->SetPitch(settings_struct.pitch);                   
  sam->SetThroat(settings_struct.throat);                  
  sam->SetMouth(settings_struct.mouth);        
  sam->SetSingMode(settings_struct.singmode);              
  sam->SetPhonetic(settings_struct.phonetic);
}
