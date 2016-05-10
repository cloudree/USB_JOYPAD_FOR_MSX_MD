/*
  MSX Joystick from USBHIDJoystick (PC/PS3BT/XBOXRECV)
  modified by cloudree
*/

/*
  Example sketch for the PS3 Bluetooth library - developed by Kristian Lauszus
  For more information visit my blog: http://blog.tkjelectronics.dk/ or
  send me an e-mail:  kristianl@tkjelectronics.com
*/
/*
  Example sketch for the Xbox Wireless Reciver library - developed by Kristian Lauszus
  It supports up to four controllers wirelessly
  For more information see the blog post: http://blog.tkjelectronics.dk/2012/12/xbox-360-receiver-added-to-the-usb-host-library/ or
  send me an e-mail:  kristianl@tkjelectronics.com
*/

// project setting

// base = 25% memory
//#define SUPPORT_PS3       // 31%
//#define SUPPORT_XBOX      // 10%
#define SUPPORT_PC        // 11%

#define _DEBUG

#ifdef SUPPORT_PC
# include <hid.h>
# include <hiduniversal.h>
#endif

#include <usbhub.h>

#ifdef SUPPORT_PS3
# include <PS3BT.h>
#endif

#ifdef SUPPORT_XBOX
# include <XBOXRECV.h>
#endif

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
# include <spi4teensy3.h>
# include <SPI.h>
#endif

USB Usb;
USBHub Hub1(&Usb); // Some dongles have a hub inside

#ifdef SUPPORT_PS3
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
PS3BT PS3(&Btd); // This will just create the instance
#endif

#ifdef SUPPORT_XBOX
XBOXRECV Xbox(&Usb);
#endif

#ifdef SUPPORT_PC
HIDUniversal Hid(&Usb);
#endif

const int PIN_DEVICE  = 7;    // Joytron Check = LOW, / Wireless = HIGH_PULL up (off)

const int PIN_UP      = 2;    // MD up
const int PIN_DOWN    = 3;    // MD down
const int PIN_LEFT    = 4;    // MD left
const int PIN_RIGHT   = 5;    // MD right
const int PIN_START   = 6;    // MD Start

const int PIN_A       = A5;    // A
const int PIN_B       = A4;    // B
const int PIN_C       = A3;    // C
const int PIN_X       = A2;    // X
const int PIN_Y       = A1;    // Y
const int PIN_Z       = A0;    // Z

volatile bool isUp, isDown, isLeft, isRight; 
volatile bool isA, isB, isC, isD, isE, isF, isG, isH;
volatile bool isStart, isSelect;

// ----------------------------------
void DBG( String msg )
{
#ifdef _DEBUG
  Serial.println( msg );
#endif
}

// ----------------------------------
#ifdef SUPPORT_PC
class JoystickReportParser : public HIDReportParser {
  public:
    JoystickReportParser() { }
    virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
    {
      if( digitalRead( PIN_DEVICE ) == HIGH ) {
        // Standard Wireless
        isUp =    ( (int8_t)buf[9] > 0 ) || ( buf[2] & 0x01 );    if( isUp ) DBG("UP");
        isDown =  ( (int8_t)buf[9] < 0 ) || ( buf[2] & 0x02 );    if( isDown ) DBG("DN");
        isLeft =  ( (int8_t)buf[7] < 0 ) || ( buf[2] & 0x04 );    if( isLeft ) DBG("LT");
        isRight = ( (int8_t)buf[7] > 0 ) || ( buf[2] & 0x08 );    if( isRight ) DBG("RT");
        isA = ( buf[3] & 0x10 );    if( isA ) DBG("A");
        isB = ( buf[3] & 0x20 );    if( isB ) DBG("B");
        isC = ( buf[3] & 0x40 );    if( isC ) DBG("C");
        isD = ( buf[3] & 0x80 );    if( isD ) DBG("D");
        isE = ( buf[3] & 0x01 );    if( isE ) DBG("E");
        isF = ( buf[3] & 0x02 );    if( isF ) DBG("F");
        isStart = ( buf[2] & 0x10 );    if( isStart ) DBG("Start");
        isSelect = ( buf[2] & 0x20 );   if( isSelect ) DBG("Select");
      }
      else {
        // Joytron Pae-Wang
        isUp =    ( buf[2] == 0x07 || buf[2] == 0x00 || buf[2] == 0x01 );    if( isUp ) DBG("UP");
        isRight = ( buf[2] == 0x01 || buf[2] == 0x02 || buf[2] == 0x03 );    if( isRight ) DBG("RT");
        isDown =  ( buf[2] == 0x03 || buf[2] == 0x04 || buf[2] == 0x05 );    if( isDown ) DBG("DN");
        isLeft =  ( buf[2] == 0x05 || buf[2] == 0x06 || buf[2] == 0x07 );    if( isLeft ) DBG("LT");
        //  10 01 08 20
        //  40 02 04 80        
        isA = ( buf[0] & 0x40 );    if( isA ) DBG("A");
        isB = ( buf[0] & 0x02 );    if( isB ) DBG("B");
        isC = ( buf[0] & 0x04 );    if( isC ) DBG("C");
        isD = ( buf[0] & 0x10 );    if( isD ) DBG("D");
        isE = ( buf[0] & 0x01 );    if( isE ) DBG("E");
        isF = ( buf[0] & 0x08 );    if( isF ) DBG("F");
        //isG = ( buf[0] & 0x20 );    if( isG ) DBG("G");
        //isH = ( buf[0] & 0x80 );    if( isH ) DBG("H");
        isStart = ( buf[1] & 0x02 );    if( isStart ) DBG("Start");
        isSelect = ( buf[1] & 0x01 );   if( isSelect ) DBG("Select");        
      }
    }
} Joy;
#endif

// ----------------------------------
void setup()
{
#ifdef _DEBUG
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

  Serial.println( "PS3 Bluetooth / X-Box Wireless Receiver / PC USB Wireless controller for MSX by cloudree" );
  if ( Usb.Init() == -1 ) {
    Serial.println( "No USB Host Shield Found" );
    while (1); //halt
  }
#endif

  // usage define
  pinMode( PIN_DEVICE, INPUT_PULLUP);

  pinMode( PIN_UP,     INPUT);
  pinMode( PIN_DOWN,   INPUT);
  pinMode( PIN_LEFT,   INPUT);
  pinMode( PIN_RIGHT,  INPUT);
  pinMode( PIN_START,  INPUT);
  pinMode( PIN_A,      INPUT);
  pinMode( PIN_B,      INPUT);
  pinMode( PIN_C,      INPUT);
  pinMode( PIN_X,      INPUT);
  pinMode( PIN_Y,      INPUT);
  pinMode( PIN_Z,      INPUT);

  // initialize
  digitalWrite(PIN_UP,    LOW);
  digitalWrite(PIN_DOWN,  LOW);
  digitalWrite(PIN_LEFT,  LOW);
  digitalWrite(PIN_RIGHT, LOW);
  digitalWrite(PIN_START, LOW);
  digitalWrite(PIN_A,     LOW);
  digitalWrite(PIN_B,     LOW);
  digitalWrite(PIN_C,     LOW);
  digitalWrite(PIN_X,     LOW);
  digitalWrite(PIN_Y,     LOW);
  digitalWrite(PIN_Z,     LOW);

  isUp = isDown = isLeft = isRight = isA = isB = isC = isD = isE = isF = false;

#ifdef SUPPORT_PC
  delay(200);
  if ( !Hid.SetReportParser(0, &Joy) )
    DBG( "SetReportParser Error" );
#endif
}

void loop()
{
  Usb.Task();

#ifdef SUPPORT_PS3
  if (PS3.PS3Connected || PS3.PS3NavigationConnected)
  {
    isUp = ( PS3.getAnalogHat(LeftHatY) < 117 || PS3.getButtonPress(UP) );
    isDown = ( PS3.getAnalogHat(LeftHatY) > 137 || PS3.getButtonPress(DOWN) );
    isLeft = ( PS3.getAnalogHat(LeftHatX) < 117 || PS3.getButtonPress(LEFT) );
    isRight = ( PS3.getAnalogHat(LeftHatX) > 137 || PS3.getButtonPress(RIGHT) );
    isA = ( PS3.getButtonPress(CROSS) );
    isB = ( PS3.getButtonPress(CIRCLE) );
    isC = ( PS3.getButtonPress(SQUARE) );
    isD = ( PS3.getButtonPress(TRIANGLE) );
    isE = ( PS3.getButtonPress(L1) );
    isF = ( PS3.getButtonPress(R1) );
    isStart = ( PS3.getButtonPress(START) );
    isSelect = ( PS3.getButtonPress(SELECT) );
  }
#endif

#ifdef SUPPORT_XBOX
  int devID = 0;
  if (Xbox.XboxReceiverConnected && Xbox.Xbox360Connected[devID])
  {
    isUp = ( Xbox.getAnalogHat(LeftHatY, devID) > 7500 || Xbox.getButtonPress(UP, devID) );
    isDown = ( Xbox.getAnalogHat(LeftHatY, devID) < -7500 || Xbox.getButtonPress(DOWN, devID) );
    isLeft = ( Xbox.getAnalogHat(LeftHatX, devID) < -7500 || Xbox.getButtonPress(LEFT, devID) );
    isRight = ( Xbox.getAnalogHat(LeftHatX, devID) > 7500 || Xbox.getButtonPress(RIGHT, devID) );
    isA = ( Xbox.getButtonPress(A, devID) );
    isB = ( Xbox.getButtonPress(B, devID) );
    isC = ( Xbox.getButtonPress(X, devID) );
    isD = ( Xbox.getButtonPress(Y, devID) );
    isE = ( Xbox.getButtonPress(L1, devID) );
    isF = ( Xbox.getButtonPress(R1, devID) );
    isStart = ( Xbox.getButtonPress(START, devID) );
    isSelect = ( Xbox.getButtonPress(SELECT, devID) );
  }
#endif

  pinMode( PIN_UP, isUp ? OUTPUT : INPUT);
  pinMode( PIN_DOWN, isDown ? OUTPUT : INPUT);
  pinMode( PIN_LEFT, isLeft ? OUTPUT : INPUT);
  pinMode( PIN_RIGHT, isRight ? OUTPUT : INPUT);
  pinMode( PIN_START, isStart ? OUTPUT : INPUT);
  pinMode( PIN_A, isA ? OUTPUT : INPUT);
  pinMode( PIN_B, isB ? OUTPUT : INPUT);
  pinMode( PIN_C, isC ? OUTPUT : INPUT);
  pinMode( PIN_X, isD ? OUTPUT : INPUT);
  pinMode( PIN_Y, isE ? OUTPUT : INPUT);
  pinMode( PIN_Z, isF ? OUTPUT : INPUT);
}

