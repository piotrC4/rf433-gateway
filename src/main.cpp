/*
 * Based on:
 * https://github.com/mhaack/mqtt-433mhz-gateway-homie
 */

#include <Homie.h>
#include <RCSwitch.h>
#include <EEPROM.h>

// #define NODE_FIRMWARE "rf433-transceiver"
// #define NODE_VERSION "0.04"
#define PIN_TRANSMITER D0
#define PIN_RESET 4
#define PIN_RECEIVER D5

String mappingConfig;
// Mappings format:
// "o-1:[139400];o-2:[139707];o-3:[1398097,1398100];o-4:[139803];i-1:[44618];i-2:[44620];i-3:[44623];i-4:[44638];i-5:[44700];“
// gang:[3952904];

long previousData = 0;
unsigned long int startMomentReceive = 0; // znacznik czasu odczytu analoga

// RF Switch
RCSwitch mySwitch = RCSwitch();

HomieNode rfSwitchNode("MQTTto433", "rf transmitter");
HomieNode rfReceiverNode("433toMQTT", "rf receiver");

/***************************************************************
 * Map code to channel
 */
String getChannelByCode(const String & currentCode)
{
  String mapping = "";
  String codes = "";
  int lastIndex = 0;
  int lastCodeIndex = 0;

  for (unsigned int i = 0; i < mappingConfig.length(); i++) {
    if (mappingConfig.substring(i, i + 1) == ";") {
      mapping = mappingConfig.substring(lastIndex, i);
      codes = mapping.substring(mapping.indexOf(':') + 2, mapping.length() - 1);
      for (unsigned int j = 0; j < codes.length(); j++) {
        if (codes.substring(j, j + 1) == ",") {
          if (currentCode.indexOf(codes.substring(lastCodeIndex, j)) > -1) {
            return mapping.substring(0, mapping.indexOf(':'));
          }
          lastCodeIndex = j+1;
          //codes = codes.substring(j + 1, codes.length());
        }
      }
      if (currentCode.indexOf(codes) > -1) {
        return mapping.substring(0, mapping.indexOf(':'));;
      }
      lastIndex = i + 1;
    }
  }
  return "0";
}

/***************************************************************
 * Inside homie loop handler
 */
void loopHandler()
{
  if (mySwitch.available())
  {
    long data = mySwitch.getReceivedValue();
    mySwitch.resetAvailable();

    if (data != previousData || millis()-startMomentReceive > 500)
    {
      previousData = data;
      Serial.println( String("Receiving 433Mhz > MQTT signal: ") + data);

      String currentCode = String(data);
      String channelId = getChannelByCode(currentCode);
      Serial.println( String("Code: ")+ currentCode+" matched to channel:" + channelId );
      rfReceiverNode.setProperty( String("channel-")+channelId).send( currentCode );
    }
    startMomentReceive = millis();
  }
}

/***************************************************************
 *
 */
bool rfSwitchOnHandler(const HomieRange& range, const String& value) {
  long int data = 0;
  int pulseLength = 350;
  if (value.indexOf(',') > 0) {
    pulseLength = atoi(value.substring(0, value.indexOf(',')).c_str());
    data = atoi(value.substring(value.indexOf(',') + 1).c_str());
  } else {
    data = atoi(value.c_str());
  }
  Serial.println( String("Receiving MQTT > 433Mhz signal: ") + pulseLength + ":" + data);

  mySwitch.setPulseLength(pulseLength);
  mySwitch.send(data, 24);
  rfSwitchNode.setProperty( "on").send( String(data) );
  return true;
}

/***************************************************************
 *
 */
bool rfReceiverMappingHandler(const HomieRange& range, const String& value)
{
  if (value=="get")
  {
    rfReceiverNode.setProperty("mappings").send( mappingConfig);
    return true;
  }
  mappingConfig = value;
  rfReceiverNode.setProperty("mappings").send( mappingConfig);
  File configFile = SPIFFS.open("/mappings.json", "w");
  if (configFile)
  {
    configFile.println(mappingConfig);
    configFile.close();
  }
  return true;
}

/***************************************************************
 * Homie event
 */
void homieEventHandler(const HomieEvent& event) {
  switch(event.type) {
    case HomieEventType::STANDALONE_MODE:
      // Standalone mode is started
      Serial.println("** HOMIE STANDALONE MODE");
      break;
    case HomieEventType::CONFIGURATION_MODE:
      // Configuration mode is started
      Serial.println("** HOMIE CONFIG MODE");
      break;
    case HomieEventType::NORMAL_MODE:
      // Normal mode is started
      Serial.println("** HOMIE NORMAL MODE");
      break;
    case HomieEventType::OTA_STARTED:
      // OTA mode is started
      Serial.println("** HOMIE OTA MODE");
      break;
    case HomieEventType::OTA_SUCCESSFUL:
      // OTA sucessfull
      Serial.println("** HOMIE OTA SUCESSFUL");
      break;
    case HomieEventType::OTA_FAILED:
      // OTA failed
      Serial.println("** HOMIE OTA FAILED");
      break;
    case HomieEventType::OTA_PROGRESS:
      Serial.println("** HOMIE OTA PROGRESS");
      break;
    case HomieEventType::ABOUT_TO_RESET:
      Serial.println("** HOMIE ABOUT TO RESET");
      // The device is about to reset
      break;
    case HomieEventType::WIFI_CONNECTED:
      // Wi-Fi is connected in normal mode
      Serial.println("** HOMIE WIFI CONNECTED");
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      // Wi-Fi is disconnected in normal mode
      Serial.println("** HOMIE WIFI DISCONNECTED");
      // ESP.restart();
      break;
    case HomieEventType::MQTT_READY:
      // MQTT is connected in normal mode
      Serial.println("** HOMIE MQTT READY");
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      // MQTT is disconnected in normal mode
      Serial.println("** HOMIE MQTT DISCONNECTED");
      // ESP.restart();
      break;
    case HomieEventType::MQTT_PACKET_ACKNOWLEDGED:
      // MQTT packet acknowledged
      Serial.println("** HOMIE MQTT PACKET ACKNOWLEDGED");
      break;
    case HomieEventType::READY_TO_SLEEP:
      // Ready to sleep
      Serial.println("** HOMIE READY TO SLEEP");
      break;
    case HomieEventType::SENDING_STATISTICS:
      // Sending statistics
      Serial.println("** HOMIE SENDING STATISTICS");
      break;
  }
}

/***************************************************************
 *
 */
void setupHandler()
{
  rfReceiverNode.setProperty("mappings").send( mappingConfig);
}

/***************************************************************
 *
 */
void setup()
{
  Serial.begin(115200);


  if (SPIFFS.begin())
  {
    if (SPIFFS.exists("/mappings.json"))
    {
      File configFile = SPIFFS.open("/mappings.json", "r");
      if (configFile)
      {
        mappingConfig = configFile.readStringUntil('\n');
      }
    }
  }
  // init RF library
  mySwitch.enableTransmit(PIN_TRANSMITER);
  mySwitch.setRepeatTransmit(20); //increase transmit repeat to avoid lost of rf sendings
  mySwitch.enableReceive(PIN_RECEIVER);

  Homie_setFirmware(FIRMWARE_NAME, FIRMWARE_VER);
  Homie_setBrand("MyIOT");
  Homie.disableLedFeedback();
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.disableLogging();

  rfSwitchNode.advertise("on").settable(rfSwitchOnHandler);
  rfReceiverNode.advertise("mappings").settable(rfReceiverMappingHandler);

  Homie.setResetTrigger(PIN_RESET, LOW, 10000);
  Homie.onEvent(homieEventHandler);
  Homie.setup();
}

/***************************************************************
 *
 */
void loop()
{
  Homie.loop();
}
