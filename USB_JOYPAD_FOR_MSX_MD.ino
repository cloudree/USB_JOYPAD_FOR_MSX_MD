/*
  MSX Joystick from USBHIDJoystick (PC/PS3BT/XBOXRECV)
  modified by cloudree

  2016.10.16 : arduino 1.6.10 + USB host shield library 2.0 : circuit@home.org
  2016.07.30 : mouse support
  MSX mouse specs are ...
    https://www.msx.org/wiki/Mouse/Trackball
    http://www.faq.msxnet.org/connector.html
    https://www.msx.org/forum/msx-talk/hardware/use-10eu-connect-modern-mouse-msx
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
//#define SUPPORT_PS3       // PS3 joypad : 31%
//#define SUPPORT_XBOX      // Xbox joypad : 10%
#define SUPPORT_PC          // PC USB Joystick : 11%
//#define SUPPORT_MOUSE     // PC Mouse
#define _DEBUG
#define USBHOSTLIB20      // arduino 1.6.10 + USB host shield library 2.0 : circuit@home.org

#if defined SUPPORT_PC || defined SUPPORT_MOUSE
#   ifdef USBHOSTLIB20
#       include <usbhid.h>        // arduino 1.6.10
#       define HID    USBHID
#   else
#       include <hid.h>         // arduino 1.6.x
#   endif
#   include <usbhub.h>
#   include <hiduniversal.h>
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

// USB Host Shield pin out
// D8 = GPX
// D9 = INT
// D10 = SS
// D11 = ??
// D12 = ?? with MSX
// D0~D7, D13 is Free

#ifdef SUPPORT_MOUSE
const int MAX_MOVE    = 16;

const int PIN_D0      = 2;    // data0(P1)
const int PIN_D1      = 3;    // data1(P2)
const int PIN_D2      = 4;    // data2(P3)
const int PIN_D3      = 5;    // data3(P4)

const int PIN_STROBE  = 7;    // OUT(P8) from MSX/X68K

const int PIN_LCLK    = A5;   // Left Click(P6)
const int PIN_RCLK    = A4;   // Right Click(P7)

volatile byte moveX, moveY;
volatile bool pressLeft, pressRight;
volatile int outCount, lastStrobe;
#else
const int PIN_UP      = 2;    // Up
const int PIN_DOWN    = 3;    // Down
const int PIN_LEFT    = 4;    // Left
const int PIN_RIGHT   = 5;    // Right
const int PIN_START   = 6;    // Start
const int PIN_SELECT  = 7;    // Select

const int PIN_A       = A5;    // A
const int PIN_B       = A4;    // B
const int PIN_C       = A3;    // C / L
const int PIN_X       = A2;    // X
const int PIN_Y       = A1;    // Y
const int PIN_Z       = A0;    // Z / R

volatile bool isUp, isDown, isLeft, isRight;
volatile bool isA, isB, isC, isX, isY, isZ;
volatile bool isStart, isSelect;
#endif

// ----------------------------------
void DBG( String msg, bool CRLF = true )
{
#ifdef _DEBUG
  if( CRLF )
    Serial.println( msg );
  else
    Serial.print( msg );
#endif
}

void DBG( byte data )
{
#ifdef _DEBUG
  Serial.print( data, HEX );
  Serial.print( " " );
#endif
}

// ----------------------------------
#if defined SUPPORT_PC || defined SUPPORT_MOUSE
class MyReportParser : public HIDReportParser {
  public:
    MyReportParser() { }
    virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
    {
      for (int i=0; i<len; i++ ) {
        PrintHex<uint8_t > (buf[i], 0);
        Serial.print(",");
      }
      Serial.println("\n");
      
#  ifdef SUPPORT_MOUSE
      moveX = (byte) buf[1];      if ( moveX ) {
        DBG( moveX );
        DBG( "=X" );
      }
      moveY = (byte) buf[2];      if ( moveY ) {
        DBG( moveY );
        DBG( "=Y" );
      }
      pressLeft = buf[0] & 1;     if ( pressLeft) DBG( "Left" );
      pressRight = buf[0] & 2;    if ( pressRight) DBG( "Right" );
#  else
      if( buf[1] == 0x14 )
      {
        // Wing Wireless
        DBG("Wing ", false);
        isUp =    ( (int8_t)buf[9] > 0 ) || ( buf[2] & 0x01 );    if ( isUp ) DBG("UP");
        isDown =  ( (int8_t)buf[9] < 0 ) || ( buf[2] & 0x02 );    if ( isDown ) DBG("DN");
        isLeft =  ( (int8_t)buf[7] < 0 ) || ( buf[2] & 0x04 );    if ( isLeft ) DBG("LT");
        isRight = ( (int8_t)buf[7] > 0 ) || ( buf[2] & 0x08 );    if ( isRight ) DBG("RT");
        isA = ( buf[3] & 0x10 );    if ( isA ) DBG("A");
        isB = ( buf[3] & 0x20 );    if ( isB ) DBG("B");
        isC = ( buf[3] & 0x40 );    if ( isC ) DBG("C");
        isX = ( buf[3] & 0x80 );    if ( isX ) DBG("X");
        isY = ( buf[3] & 0x01 );    if ( isY ) DBG("Y");
        isZ = ( buf[3] & 0x02 );    if ( isZ ) DBG("Z");
        isStart = ( buf[2] & 0x10 );    if ( isStart ) DBG("Start");
        isSelect = ( buf[2] & 0x20 );   if ( isSelect ) DBG("Select");
      }
      else if ( buf[1] == 0x80 && buf[2] == 0x80 && buf[3] == 0x80 && buf[4] == 0x80 ) {
        // PS4 : Hori Fighting Stick Mini
        DBG("PS4 ", false);
        int st = buf[5] & 0xf;
        isUp =    ( st == 7 || st <= 1 );     if ( isUp ) DBG("UP");
        isRight = ( 1 <= st && st <= 3 );     if ( isRight ) DBG("RT");
        isDown =  ( 3 <= st && st <= 5 );     if ( isDown ) DBG("DN");
        isLeft =  ( 5 <= st && st <= 7 );     if ( isLeft ) DBG("LT");
        isA = ( buf[5] & 0x20 || buf[6] & 0x04 );   if ( isA ) DBG("A");
        isB = ( buf[5] & 0x40 );                    if ( isB ) DBG("B");
        isC = ( buf[6] & 0x08 );                    if ( isC ) DBG("C");
        isX = ( buf[5] & 0x10 || buf[6] & 0x01 );   if ( isX ) DBG("X");
        isY = ( buf[5] & 0x80 );                    if ( isY ) DBG("Y");
        isZ = ( buf[6] & 0x02 );                    if ( isZ ) DBG("Z");
        isStart = ( buf[6] & 0x10 );    if ( isStart ) DBG("Start");
        isSelect = ( buf[6] & 0x20 );   if ( isSelect ) DBG("Select");
      }
      else if ( buf[2] == 0x80 && buf[3] == 0x80 && buf[4] == 0x80 ) {
        // NES copy USB pad
        DBG("NES-USB ", false);
        isUp =    ( buf[1] == 0 );      if ( isUp ) DBG("UP");
        isDown =  ( buf[1] == 0xff );   if ( isDown ) DBG("DN");
        isLeft =  ( buf[0] == 0 );      if ( isLeft ) DBG("LT");
        isRight = ( buf[0] == 0xff );   if ( isRight ) DBG("RT");
        isA = ( buf[5] & 0x20 );    if ( isA ) DBG("A");
        isB = ( buf[5] & 0x40 );    if ( isB ) DBG("B");
        isC = ( buf[6] & 0x01 );    if ( isC ) DBG("C");
        isX = ( buf[5] & 0x10 );    if ( isX ) DBG("X");
        isY = ( buf[5] & 0x80 );    if ( isY ) DBG("Y");
        isZ = ( buf[6] & 0x02 );    if ( isZ ) DBG("Z");
        isStart = ( buf[6] & 0x20 );    if ( isStart ) DBG("Start");
        isSelect = ( buf[6] & 0x10 );   if ( isSelect ) DBG("Select");
      }
      else if ( (buf[3] & 0xF0) == 0xF0 )
      {
        // Dahoon DHU-3300
        DBG("DHU-3300 ", false);
        isUp =    ( buf[1] == 0 );        if ( isUp ) DBG("UP");
        isDown =  ( buf[1] == 0xff );     if ( isDown ) DBG("DN");
        isLeft =  ( buf[0] == 0 );        if ( isLeft ) DBG("LT");
        isRight = ( buf[0] == 0xff );     if ( isRight ) DBG("RT");
        isA = ( buf[2] & 0x01 ) || ( buf[2] & 0x08 );    if ( isA ) DBG("A");
        isB = ( buf[2] & 0x02 );    if ( isB ) DBG("B");
        isC = ( buf[2] & 0x04 );    if ( isC ) DBG("C");
        isX = ( buf[2] & 0x10 ) || ( buf[2] & 0x80 );    if ( isX ) DBG("X");
        isY = ( buf[2] & 0x20 );    if ( isY ) DBG("Y");
        isZ = ( buf[2] & 0x40 );    if ( isZ ) DBG("Z");
        isStart = ( buf[3] & 0x01 );    if ( isStart ) DBG("Start");
        isSelect = ( buf[3] & 0x02 );   if ( isSelect ) DBG("Select");
      }
      else
      {
        // PS3 : Joytron Pae-Wang, Hori Fighting Stick Mini
        DBG("PS3 ", false);
        isUp =    ( buf[2] == 0x07 || buf[2] == 0x00 || buf[2] == 0x01 );    if ( isUp ) DBG("UP");
        isRight = ( buf[2] == 0x01 || buf[2] == 0x02 || buf[2] == 0x03 );    if ( isRight ) DBG("RT");
        isDown =  ( buf[2] == 0x03 || buf[2] == 0x04 || buf[2] == 0x05 );    if ( isDown ) DBG("DN");
        isLeft =  ( buf[2] == 0x05 || buf[2] == 0x06 || buf[2] == 0x07 );    if ( isLeft ) DBG("LT");
        //  10 01 08 20, 01 08 20 10
        //  40 02 04 80, 02 04 80 40
        isA = ( buf[0] & 0x02 || buf[0] & 0x40 );    if ( isA ) DBG("A");
        isB = ( buf[0] & 0x04 );    if ( isB ) DBG("B");
        isC = ( buf[0] & 0x80 );    if ( isC ) DBG("C");
        isX = ( buf[0] & 0x01 || buf[0] & 0x10 );    if ( isX ) DBG("X");
        isY = ( buf[0] & 0x08 );    if ( isY ) DBG("Y");
        isZ = ( buf[0] & 0x20 );    if ( isZ ) DBG("Z");
        isStart = ( buf[1] & 0x02 );    if ( isStart ) DBG("Start");
        isSelect = ( buf[1] & 0x01 );   if ( isSelect ) DBG("Select");
      }
#  endif
    }
} ReportParser;
#endif

// ----------------------------------

USB Usb;
USBHub Hub1(&Usb); // Some dongles have a hub inside

#ifdef SUPPORT_PS3
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
PS3BT PS3(&Btd); // This will just create the instance
#endif

#ifdef SUPPORT_XBOX
XBOXRECV Xbox(&Usb);
#endif

#if defined SUPPORT_PC || defined SUPPORT_MOUSE
HIDUniversal Hid(&Usb);
#endif

void setup()
{
#ifdef _DEBUG
  Serial.begin(9600);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

  String str = "WiPad Start : ";
  str.concat (__DATE__);
  Serial.println( str );

  if ( Usb.Init() == -1 ) {
    Serial.println( "No USB Host Shield Found" );
    while (1); //halt
  }
#endif

  //ReportDescParser p;
  
  // msx pins are not HIGH / LOW input,
  // they are NOT_CONNECTED(3state, internaly pull-upped) / LOW

#ifdef SUPPORT_MOUSE
  pinMode( PIN_D0,     INPUT);
  pinMode( PIN_D1,     INPUT);
  pinMode( PIN_D2,     INPUT);
  pinMode( PIN_D3,     INPUT);
  pinMode( PIN_STROBE, INPUT);
  pinMode( PIN_LCLK,   INPUT);
  pinMode( PIN_RCLK,   INPUT);

  // initialize
  digitalWrite( PIN_D0,     LOW);
  digitalWrite( PIN_D1,     LOW);
  digitalWrite( PIN_D2,     LOW);
  digitalWrite( PIN_D3,     LOW);
  digitalWrite( PIN_LCLK,   LOW);
  digitalWrite( PIN_RCLK,   LOW);

  moveX = moveY = outCount = 0;
  pressLeft = pressRight = false;
  lastStrobe = digitalRead( PIN_STROBE);
#else
  pinMode( PIN_UP,     INPUT);
  pinMode( PIN_DOWN,   INPUT);
  pinMode( PIN_LEFT,   INPUT);
  pinMode( PIN_RIGHT,  INPUT);
  pinMode( PIN_START,  INPUT);
  pinMode( PIN_SELECT, INPUT);
  pinMode( PIN_A,      INPUT);
  pinMode( PIN_B,      INPUT);
  pinMode( PIN_C,      INPUT);
  pinMode( PIN_X,      INPUT);
  pinMode( PIN_Y,      INPUT);
  pinMode( PIN_Z,      INPUT);

  // initialize
  digitalWrite( PIN_UP,    LOW);
  digitalWrite( PIN_DOWN,  LOW);
  digitalWrite( PIN_LEFT,  LOW);
  digitalWrite( PIN_RIGHT, LOW);
  digitalWrite( PIN_START, LOW);
  digitalWrite( PIN_A,     LOW);
  digitalWrite( PIN_B,     LOW);
  digitalWrite( PIN_C,     LOW);
  digitalWrite( PIN_X,     LOW);
  digitalWrite( PIN_Y,     LOW);
  digitalWrite( PIN_Z,     LOW);

  isUp = isDown = isLeft = isRight = isA = isB = isC = isX = isY = isZ = false;
#endif

#if defined SUPPORT_PC || defined SUPPORT_MOUSE
  delay(200);
  if ( !Hid.SetReportParser(0, &ReportParser) )
    DBG( "SetReportParser Error" );
#endif
}

#ifdef SUPPORT_MOUSE
void outputNibble( byte data)
{
  digitalWrite( PIN_D0, data & 0x01 ? HIGH : LOW );
  digitalWrite( PIN_D1, data & 0x02 ? HIGH : LOW );
  digitalWrite( PIN_D2, data & 0x04 ? HIGH : LOW );
  digitalWrite( PIN_D3, data & 0x08 ? HIGH : LOW );
}

void sendMSX(char d)
{
  char c = -d / 2;
  if ( c < -MAX_MOVE ) c = -MAX_MOVE;
  if ( c > MAX_MOVE ) c = MAX_MOVE;

  long timeout = millis() + 40;

  while ( digitalRead( PIN_STROBE) == LOW) {
    if ( millis() > timeout) return;
  };
  pinMode( PIN_D3, (c & 0x80) ? INPUT : OUTPUT);
  pinMode( PIN_D2, (c & 0x40) ? INPUT : OUTPUT);
  pinMode( PIN_D1, (c & 0x20) ? INPUT : OUTPUT);
  pinMode( PIN_D0, (c & 0x10) ? INPUT : OUTPUT);

  while ( digitalRead( PIN_STROBE) == HIGH) {
    if ( millis() > timeout) return;
  };
  pinMode( PIN_D3, (c & 0x08) ? INPUT : OUTPUT);
  pinMode( PIN_D2, (c & 0x04) ? INPUT : OUTPUT);
  pinMode( PIN_D1, (c & 0x02) ? INPUT : OUTPUT);
  pinMode( PIN_D0, (c & 0x01) ? INPUT : OUTPUT);
}
#endif

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
    isStart = ( PS3.getButtonPress(START) );
    isSelect = ( PS3.getButtonPress(SELECT) );
    isA = ( PS3.getButtonPress(CROSS) );
    isB = ( PS3.getButtonPress(CIRCLE) );
    isC = ( PS3.getButtonPress(SQUARE) );
    isX = ( PS3.getButtonPress(TRIANGLE) );
    isY = ( PS3.getButtonPress(L1) );
    isZ = ( PS3.getButtonPress(R1) );
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
    isStart = ( Xbox.getButtonPress(START, devID) );
    isSelect = ( Xbox.getButtonPress(SELECT, devID) );
    isA = ( Xbox.getButtonPress(A, devID) );
    isB = ( Xbox.getButtonPress(B, devID) );
    isC = ( Xbox.getButtonPress(X, devID) );
    isX = ( Xbox.getButtonPress(Y, devID) );
    isY = ( Xbox.getButtonPress(L1, devID) );
    isZ = ( Xbox.getButtonPress(R1, devID) );
    isStart = ( Xbox.getButtonPress(START, devID) );
    isSelect = ( Xbox.getButtonPress(SELECT, devID) );
  }
#endif

#ifdef SUPPORT_MOUSE
  // Watch out the response time, think about the attachInterrupt function

  int strobe = digitalRead( PIN_STROBE);
  if ( lastStrobe != strobe )
  {
    lastStrobe = strobe;
    if ( ++outCount > 3 ) outCount = 0;
  }
  sendMSX( moveX); moveX = 0;
  sendMSX( moveY); moveY = 0;

  pinMode( PIN_LCLK, pressLeft ? OUTPUT : INPUT);
  pinMode( PIN_RCLK, pressRight ? OUTPUT : INPUT);

#else
  pinMode( PIN_UP, isUp ? OUTPUT : INPUT);
  pinMode( PIN_DOWN, isDown ? OUTPUT : INPUT);
  pinMode( PIN_LEFT, isLeft ? OUTPUT : INPUT);
  pinMode( PIN_RIGHT, isRight ? OUTPUT : INPUT);
  pinMode( PIN_START, isStart ? OUTPUT : INPUT);
  pinMode( PIN_SELECT, isSelect ? OUTPUT : INPUT);

  //
  pinMode( PIN_A, isA ? OUTPUT : INPUT);
  pinMode( PIN_B, isB ? OUTPUT : INPUT);
  pinMode( PIN_C, isC ? OUTPUT : INPUT);
  pinMode( PIN_X, isX ? OUTPUT : INPUT);
  pinMode( PIN_Y, isY ? OUTPUT : INPUT);
  pinMode( PIN_Z, isZ ? OUTPUT : INPUT);
#endif
}


