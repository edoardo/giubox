// Giubox
// RFID controller for the Squeezebox receiver

#include <EtherCard.h>

int buttonPin = 8;
int ledPin = 7;

int state = LOW;       // the current state of the led
int reading;           // the current reading from the button pin
int previous = HIGH;   // the previous reading from the button pin

long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers


// MAC Address of Ethernet Shield
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

static byte myip[] = { 10,0,1,220 };
static byte gwip[] = { 10,0,1,1 };

static byte lms_ip[] = { 10,0,1,10 };
static int lms_port = 9000;
static char lms_host[] PROGMEM = "htpc";
//static char player_id[] PROGMEM = "00:01:2e:27:19:63";
static char player_id[] PROGMEM = "00:04:20:ff:ff:01";

byte Ethernet::buffer[300];

Stash stash;

void setup () {
  Serial.begin(57600);
  Serial.println("RFID controller initialization");
  Serial.println();
  
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10))
    Serial.println( "Failed to access Ethernet controller");
    
  Serial.println();
/*
  Serial.println("Setting up DHCP");
  if (!ether.dhcpSetup())
    Serial.println("Failed to get configuration from DHCP");
*/

  ether.staticSetup(myip, gwip);

  ether.printIp("IP Address:\t", ether.myip);
  ether.printIp("Netmask:\t", ether.mymask);
  ether.printIp("Gateway:\t", ether.gwip);
  Serial.println();
  
  ether.copyIp(ether.hisip, lms_ip);
  ether.hisport = lms_port;
  
  ether.printIp("Remote address:\t", ether.hisip);
  Serial.print("Remote port:\t");
  Serial.println(ether.hisport);
  
  Serial.println();
}

void loop() {
  ether.packetLoop(ether.packetReceive());
  
  int reading = digitalRead(buttonPin);

  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
    // toggle pause on player
    sendCommand();

    if (state == HIGH)
      state = LOW;
    else
      state = HIGH;

    time = millis();
  }

  digitalWrite(ledPin, state);

  previous = reading;
}

void sendCommand() {
  Serial.print("Sending command to the server...");

  Stash::prepare(PSTR("GET /status.txt?p0=pause&player=$F HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "User-Agent: Giubox" "\r\n"
    "\r\n"),
  player_id, lms_host);
 
  ether.tcpSend();
}
