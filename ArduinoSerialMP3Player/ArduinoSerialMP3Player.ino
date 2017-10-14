/***************************************************************************************************************************/
// Demo for the Serial MP3 Player Catalex (YX5300 chip)
// Hardware: Serial MP3 Player *1
// Board:  Arduino UNO
// http://www.dx.com/p/uart-control-serial-mp3-music-player-module-for-arduino-avr-arm-pic-blue-silver-342439#.VfHyobPh5z0
//

// Uncomment SoftwareSerial for Arduino Uno or Nano.
//#define mp3 Serial3    // Connect the MP3 Serial Player to the Arduino MEGA Serial3 (14 TX3 -> RX, 15 RX3 -> TX)

#include <SoftwareSerial.h>

#define ARDUINO_RX 5  //The TX pin in module should be connected to the TX in Arduino with the following pin number
#define ARDUINO_TX 6  //The RX pin in module should be connected to the RX in Arduino with the following pin number

SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX);
												  
// Uncomment next line if you are using an Arduino Mega.
//#define mp3 Serial3    // Connect the MP3 Serial Player to the Arduino MEGA Serial3 (14 TX3 -> RX, 15 RX3 -> TX)

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

String mp3Answer;           // Answer from the MP3.

String sanswer(void);
String sbyte2hex(uint8_t b);


/******************* Command bytes **************************/
#define CMD_NEXT_SONG         0X01  //Play next song.
#define CMD_PREV_SONG         0X02  //Play previous song.
#define CMD_PLAY_W_INDEX      0X03  //Play index
#define CMD_VOLUME_UP         0X04  //Volume up
#define CMD_VOLUME_DOWN       0X05  //Volume down
#define CMD_SET_VOLUME        0X06  //set volume (0-30)
                              0X07  //Reserved, uknown function
#define CMD_SNG_CYCL_PLAY     0X08  // Single Cycle Play every song.
#define CMD_SLEEP_MODE        0X0A  //Put mp3 in sleepmode
#define CMD_WAKE_UP           0X0B  //Wake up mp3
#define CMD_RESET             0X0C  //reset mp3
#define CMD_PLAY              0X0D  //Start playing
#define CMD_PAUSE             0X0E  //Pause palying
#define CMD_PLAY_FOLDER_FILE  0X0F //Play song x out of folder y (Use hexidecimal number!!)

#define CMD_STOP_PLAY         0X16  // Stop playing continuously. 
#define CMD_FOLDER_CYCLE      0X17  // Cycle play every song in folder f 
#define CMD_SHUFFLE_PLAY      0x18  // Shuffle play (does not work currently)
#define CMD_SET_SNGL_CYCL     0X19  // Set single cycle (how many times do you want to cycle?)
#define CMD_PLAY_W_VOL        0X22  //Play song x with volume y (0-30)

//A DAC is a circuit that allows you to translate numeric values into analog signals,
//so you can have output voltages variable from 0 to 5V by setting only a variable.
//If you want to do this with an Arduino different from the Due you can't without using an external chip.
#define CMD_SET_DAC           0X1A    //Set DAC
#define DAC_ON                0X00    //ON
#define DAC_OFF               0X01    //OFF

/**************** Query's to check status ***********************/             
#define CMD_PLAYING_N         0x4C    //Is mp3 playing?
#define CMD_QUERY_STATUS      0x42    //Current status of mp3?
#define CMD_QUERY_VOLUME      0x43    //Current volume of mp3?
#define CMD_QUERY_FLDR_TRACKS 0x4e    //How many folder and tracks?
#define CMD_QUERY_TOT_TRACKS  0x48    //How many tracks?
#define CMD_QUERY_FLDR_COUNT  0x4f    //How many folder?

/********************* Select SD-Card **************************/
#define CMD_SEL_DEV           0X09  //Select sdcard
     #define DEV_TF                  0X02  //Check

/***************************************************************/

void setup(){
  Serial.begin(9600);
  mp3.begin(9600);
  delay(500);             //wait for init

  sendCommand(CMD_SEL_DEV, 0, DEV_TF);	//init sd-card and select	
  delay(500);             //wait for init
  
}

void loop(){
  char c = ' ';

  // If there a char on Serial call sendMP3Command to sendCommand
  if ( Serial.available() )
  {
    c = Serial.read();
    sendMP3Command(c);
  }

  // Check for the answer.
  if (mp3.available())
  {
    Serial.println(decodeMP3Answer());
  }
  delay(100);
}


/********************************************************************************/
/* Function sendMP3Command: seek for a 'c' command and send it to MP3           */
/* Parameter: c. Code for the MP3 Command, 'h' for help.                        */                                                                                           */
/* Return:  void                                                                */

void sendMP3Command(char c) {
		  
  switch (c) {
    case '?':
    case 'h':
      Serial.println("HELP  ");
      Serial.println(" p = Play");
      Serial.println(" P = Pause");
      Serial.println(" > = Next");
      Serial.println(" < = Previous");
      Serial.println(" s = Stop Play"); 
      Serial.println(" + = Volume UP");
      Serial.println(" - = Volume DOWN");
      Serial.println(" c = Query current file");
      Serial.println(" q = Query status");
      Serial.println(" v = Query volume");
      Serial.println(" x = Query folder count");
      Serial.println(" t = Query total file count");
      Serial.println(" f = Play folder 1.");
      Serial.println(" S = Sleep");
      Serial.println(" W = Wake up");
      Serial.println(" r = Reset");
      break;


    case 'p':
      Serial.println("Play ");
      sendCommand(CMD_PLAY);
      break;

    case 'P':
      Serial.println("Pause");
      sendCommand(CMD_PAUSE);
      break;

    case '>':
      Serial.println("Next");
      sendCommand(CMD_NEXT_SONG);
      sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
      break;

    case '<':
      Serial.println("Previous");
      sendCommand(CMD_PREV_SONG);
      sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
      break;

    case 's':
      Serial.println("Stop Play");
      sendCommand(CMD_STOP_PLAY);
      break;

    case '+':
      Serial.println("Volume Up");
      sendCommand(CMD_VOLUME_UP);
      break;

    case '-':
      Serial.println("Volume Down");
      sendCommand(CMD_VOLUME_DOWN);
      break;

    case 'c':
      Serial.println("Query current file");
      sendCommand(CMD_PLAYING_N);
      break;

    case 'q':
      Serial.println("Query status");
      sendCommand(CMD_QUERY_STATUS);
      break;

    case 'v':
      Serial.println("Query volume");
      sendCommand(CMD_QUERY_VOLUME);
      break;

    case 'x':
      Serial.println("Query folder count");
      sendCommand(CMD_QUERY_FLDR_COUNT);
      break;

    case 't':
      Serial.println("Query total file count");
      sendCommand(CMD_QUERY_TOT_TRACKS);
      break;

    case 'f':
      Serial.println("Playing folder 1");
      sendCommand(CMD_FOLDER_CYCLE, 1, 0);
      break;

    case 'S':
      Serial.println("Sleep");
      sendCommand(CMD_SLEEP_MODE);
      break;
      
    case 'W':
      Serial.println("Wake up");
      sendCommand(CMD_WAKE_UP);
      break;

    case 'r':
      Serial.println("Reset");
      sendCommand(CMD_RESET);
      break;
  }
}


/********************************************************************************/
/*Function decodeMP3Answer: Decode MP3 answer.                                  */
/*Parameter:-void                                                               */
/*Return: The answer of the mp3                                                 */

String decodeMP3Answer() {
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();

  switch (ansbuf[3]) {
    case 0x3A:
      decodedMP3Answer += " -> Memory card inserted.";
      break;

    case 0x3D:
      decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
      break;

    case 0x40:
      decodedMP3Answer += " -> Error";
      break;

    case 0x41:
      decodedMP3Answer += " -> Data received correctly. ";
      break;

    case 0x42:
      decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
      break;

    case 0x48:
      decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
      break;

    case 0x4C:
      decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
      break;

    case 0x4E:
      decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
      break;

    case 0x4F:
      decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
      break;
  }

  return decodedMP3Answer;
}


/********************************************************************************/
/*Function: Send command to the MP3                                             */
/*Parameter: byte command                                                       */
/*Parameter: byte dat1 parameter for the command                                */
/*Parameter: byte dat2 parameter for the command                                */

void sendCommand(byte command){
  sendCommand(command, 0, 0);
}

void sendCommand(byte command, byte dat1, byte dat2){
  delay(20);
  Send_buf[0] = 0x7E;    //Open command
  Send_buf[1] = 0xFF;    //Version info
  Send_buf[2] = 0x06;    //Length of command excluding the open an closing bytes
  Send_buf[3] = command; //Command
  Send_buf[4] = 0x01;    //0x00 NO, 0x01 feedback (what does this actually do?
  Send_buf[5] = dat1;    //datah
  Send_buf[6] = dat2;    //datal
  Send_buf[7] = 0xEF;    //Close command
  Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++)  {
    mp3.write(Send_buf[i]) ;
    Serial.print(sbyte2hex(Send_buf[i]));
  }
  Serial.println();       //Print white line
}



/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                       */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */


String sbyte2hex(uint8_t b){
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}


/********************************************************************************/
/*Function: shex2int. Returns a int from an HEX string.                         */
/*Parameter: s. char *s to convert to HEX.                                      */
/*Parameter: n. char *s' length.                                                */
/*Return: int                                                                   */

int shex2int(char *s, int n){
  int r = 0;
  for (int i=0; i<n; i++){
     if(s[i]>='0' && s[i]<='9'){
      r *= 16; 
      r +=s[i]-'0';
     }else if(s[i]>='A' && s[i]<='F'){
      r *= 16;
      r += (s[i] - 'A') + 10;
     }
  }
  return r;
}


/********************************************************************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */

String sanswer(void){
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (mp3.available() && (i < 10))  {
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))  {
    return mp3answer;
  }

  return "???: " + mp3answer;
}
