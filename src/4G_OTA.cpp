/************************************************************4G_OTA***************************************************************/
#include "4G_OTA.h"
#include "Variants.h"

TinyGsm ota_modem(SerialAT);
TinyGsmClient ota_client(ota_modem);

uint8_t gu8_ota_update_available = 0;
extern String CP_Id_m;

#if 1


#if 1
// const char resource[] = "/evse_test_ota.php";
const char resource[] = "/evse_bm_7_4kw_ota.php";
const char server[] = "34.100.138.28";
const int port = 80;
#endif

// const char gprsUser[] = "";
// const char gprsPass[] = "";

uint32_t knownCRC32 = 0x6f50d767;
uint32_t knownFileSize = 1148544;

// extern TinyGsm modem;
/// extern TinyGsmClient client;

// extern TinyGsm modem(Serial2);
// extern TinyGsmClient client(modem);

extern uint8_t gu8_OTA_update_flag;

String APN = "m2misafe";
char g_apn[8] = { 'm', '2', 'm', 'i', 's', 'a', 'f', 'e' };
// char g_apn[14] = {'a', 'i', 'r', 't', 'e', 'l', 'g', 'p', 'r', 's', '.', 'c', 'o', 'm'};


void OTA_4G_setup_4G_OTA_get(void)
{
  // modem.gprsConnect(apn, gprsUser, gprsPass);
  // APN = getAPN();

  // strcpy(g_apn,APN.c_str());
  // setup4G();
  OTA_4G_setup4G();
  Serial.print("Waiting for network...");
  if (!ota_modem.waitForNetwork())
  {
    Serial.println(" fail");

#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isNetworkConnected())
  {
    Serial.println("Network connected");
  }
#if TINY_GSM_USE_GPRS | 1
  // GPRS connection parameters are usually set after network registration
  Serial.print("Connecting to ");
  Serial.print(g_apn);
  if (!ota_modem.gprsConnect(g_apn, gprsUser, gprsPass))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isGprsConnected())
  {
    Serial.println("GPRS connected");
  }
#endif
  Serial.print("Connecting to ");

  Serial.print(server);

  // strcpy(&fota_host[0], host_fota.c_str());
  // strcpy(&fota_path[0], path_fota.c_str());
  // fota_port = port_fota;

  // Serial.print(fota_host);

  if (!ota_client.connect(server, port))
    // if (!ota_client.connect(fota_host, fota_port))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  // ota_client.print(String("POST ") + resource + " HTTP/1.0\r\n");
  ota_client.print(String("GET ") + resource + " HTTP/1.1\r\n");
  ota_client.print(String("Host: ") + server + "\r\n");

  // dynamic update
  //  ota_client.print(String("GET ") + fota_path + " HTTP/1.1\r\n");
  //  ota_client.print(String("Host: ") + fota_host + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(WiFi.macAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-AP-MAC: ") + String(WiFi.softAPmacAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-sketch-md5: ") + String(ESP.getSketchMD5()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getSdkVersion()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getFreeSketchSpace()) + "\r\n");

  // ota_client.print(String("x-ESP32-firmware-version: ") + VERSION + "\r\n");
  // ota_client.print(String("x-ESP32-device-id: ") + "evse_001" + "\r\n");

  ota_client.print(String("x-ESP32-device-id: ") + CP_Id_m + "\r\n");
  ota_client.print(String("x-ESP32-firmware-version: ") + EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION + "\r\n");

  ota_client.print("Connection: close\r\n\r\n");

  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();
  Serial.println("Waiting for response header");

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the ota_client, we need to exit.

  const uint32_t ota_clientReadTimeout = 600000;
  uint32_t ota_clientReadStartTime = millis();
  String headerBuffer;
  bool finishedHeader = false;
  uint32_t contentLength = 0;

  while (!finishedHeader)
  {
    int nlPos;

    if (ota_client.available())
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        char c = ota_client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        if (c < 16)
          Serial.print('0');
        Serial.print(c, HEX);
        Serial.print(' ');
        /*if (isprint(c))
          Serial.print(reinterpret_cast<char> c);
          else
          Serial.print('*');*/
        Serial.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0)
          break;
      }
    }
    else
    {
      if (millis() - ota_clientReadStartTime > ota_clientReadTimeout)
      {
        // Time-out waiting for data from ota_client
        Serial.println(">>> ota_client Timeout !");
        break;
      }
    }
    // See if we have a new line.
    nlPos = headerBuffer.indexOf(F("\r\n"));

    if (nlPos > 0)
    {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(F("content-length:")))
      {
        contentLength = headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        // Serial.print(F("Got Content Length: "));  // uncomment for
        // Serial.println(contentLength);            // confirmation
      }

      headerBuffer.remove(0, nlPos + 2); // remove the line
    }
    else if (nlPos == 0)
    {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line),
      // we are done with the header.
      finishedHeader = true;
    }
  }

  // The two cases which are not managed properly are as follows:
  // 1. The ota_client doesn't provide data quickly enough to keep up with this
  // loop.
  // 2. If the ota_client data is segmented in the middle of the 'Content-Length: '
  // header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32 crc;
  // File file = FFat.open("/update.bin", FILE_APPEND);
  // if (finishedHeader && contentLength == knownFileSize) {
  if (finishedHeader)
  {

    Serial.println("Reading response data");
    ota_clientReadStartTime = millis();

    String get_response = "";
    // printPercent(readLength, contentLength);
    while (readLength < contentLength && ota_client.connected() &&
      millis() - ota_clientReadStartTime < ota_clientReadTimeout)
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        // uint8_t c = ota_client.read();
        char c = ota_client.read();
        if (c == 'f')
        {
          Serial.print("no OTA update");
          gu8_ota_update_available = 0;
          gu8_OTA_update_flag = 3;
        }
        if (c == 't')
        {
          Serial.print("OTA update available");
          gu8_ota_update_available = 1;
          gu8_OTA_update_flag = 2;
        }
        // Serial.print(c);
        get_response += c;
        // Serial.print(reinterpret_cast<char>c);  // Uncomment this to show
        // data
        // crc.update(c);
        readLength++;
      }
      Serial.println("\r\n" + get_response);

      if (get_response.equals("true") == true)
      {
        // gu8_OTA_update_flag = 2;
        // Serial.print("OTA update available");
      }
      else if (get_response.equals("false") == false)
      {
        // gu8_OTA_update_flag = 3;
        // Serial.print("no OTA update");
      }
      get_response = "";
      Serial.println("\r\nota_client disconnected....");
    }
    // printPercent(readLength, contentLength);
  }

  timeElapsed = millis() - timeElapsed;
  Serial.println();

  ota_client.stop();
  Serial.println("Server disconnected");

#if TINY_GSM_USE_GPRS
  ota_modem.gprsDisconnect();
  Serial.println("GPRS disconnected");
#endif

  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");
  Serial.println(contentLength);
  Serial.print("Actually read:  ");
  Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x");
  Serial.println(crc.finalize(), HEX);
  Serial.print("Known CRC32:    0x");
  Serial.println(knownCRC32, HEX);
  Serial.print("Duration:       ");
  Serial.print(duration);
  Serial.println("s");
}
///////////////////////////get_response///////////////////

//------------------------------------------ 4G OTA ----------------------------------------//
void OTA_4G_setup_4G_OTA(void)
{
  // strcpy(g_apn,APN.c_str());
  OTA_4G_setup4G();
  Serial.print("Waiting for network...");
  if (!ota_modem.waitForNetwork())
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isNetworkConnected())
  {
    Serial.println("Network connected");
  }
#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
  Serial.print("Connecting to ");
  Serial.print(g_apn);
  if (!ota_modem.gprsConnect(g_apn, gprsUser, gprsPass))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isGprsConnected())
  {
    Serial.println("GPRS connected");
  }
#endif
  Serial.print("Connecting to ");
  Serial.print(server);
  // Serial.print(fota_host);

  if (!ota_client.connect(server, port))
    // if (!ota_client.connect(fota_host, fota_port))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");
  // int sketch_md5=ESP.getSketchMD5();
  // Serial.println("sketch_md5 =" + String(sketch_md5));
  //  Make a HTTP GET request:

  ota_client.print(String("POST ") + resource + " HTTP/1.1\r\n");
  ota_client.print(String("Host: ") + server + "\r\n");
  // ota_client.print(String("POST ") + fota_path + " HTTP/1.1\r\n");
  // ota_client.print(String("Host: ") + fota_host + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(WiFi.macAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-AP-MAC: ") + String(WiFi.softAPmacAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-sketch-md5: ") + String(ESP.getSketchMD5()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getSdkVersion()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getFreeSketchSpace()) + "\r\n");

  // ota_client.print(String("x-ESP32-firmware-version: ") + VERSION + "\r\n");
  // ota_client.print(String("x-ESP32-device-id: ") + "evse_001" + "\r\n");
  ota_client.print(String("x-ESP32-device-id: ") + CP_Id_m + "\r\n");
  ota_client.print(String("x-ESP32-firmware-version: ") + EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION + "\r\n");

  ota_client.print("Connection: close\r\n\r\n");

  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();
  Serial.println("Waiting for response header");

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the ota_client, we need to exit.

  const uint32_t ota_clientReadTimeout = 600000;
  uint32_t ota_clientReadStartTime = millis();
  String headerBuffer;
  bool finishedHeader = false;
  uint32_t contentLength = 0;

  while (!finishedHeader)
  {
    int nlPos;

    if (ota_client.available())
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        char c = ota_client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        if (c < 16)
          Serial.print('0');
        Serial.print(c, HEX);
        Serial.print(' ');
        /*if (isprint(c))
          Serial.print(reinterpret_cast<char> c);
          else
          Serial.print('*');*/
        Serial.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0)
          break;
      }
    }
    else
    {
      if (millis() - ota_clientReadStartTime > ota_clientReadTimeout)
      {
        // Time-out waiting for data from ota_client
        Serial.println(">>> ota_client Timeout !");
        break;
      }
    }
    // See if we have a new line.
    nlPos = headerBuffer.indexOf(F("\r\n"));

    if (nlPos > 0)
    {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(F("content-length:")))
      {
        contentLength = headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        Serial.print("Got Content Length: "); // uncomment for
        Serial.println(contentLength);        // confirmation
        // if(contentLength <= 0)
        // {
        //   // Serial.print(F("Got Content Length: "));  // comment at 06092023_170900
        //   // Serial.println(contentLength);
        //   contentLength = 1;
        // }
      }

      headerBuffer.remove(0, nlPos + 2); // remove the line
    }
    else if (nlPos == 0)
    {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line),
      // we are done with the header.
      finishedHeader = true;
    }
  }

  // The two cases which are not managed properly are as follows:
  // 1. The ota_client doesn't provide data quickly enough to keep up with this
  // loop.
  // 2. If the ota_client data is segmented in the middle of the 'Content-Length: '
  // header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32 crc;

#if 1
  File fs = FFat.open("/update.bin", FILE_APPEND);
  if (FFat.remove("/update.bin"))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
#endif


  File file = FFat.open("/update.bin", FILE_APPEND);
  // if (finishedHeader && contentLength == knownFileSize) {
  if (finishedHeader)
  {

    Serial.println("Reading response data");
    ota_clientReadStartTime = millis();

    OTA_4G_printPercent(readLength, contentLength);
    while (readLength < contentLength && ota_client.connected() &&
      millis() - ota_clientReadStartTime < ota_clientReadTimeout)
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        uint8_t c = ota_client.read();
        // char c = ota_client.read();
        // Serial.print(c);
        if (!file.write(c))
        {
          Serial.println("not Appending file");
        }

        // Serial.print(reinterpret_cast<char>c);  // Uncomment this to show
        // data
        crc.update(c);
        readLength++;
        if (readLength % (contentLength / 100) == 0)
        {
          OTA_4G_printPercent(readLength, contentLength);
          Serial.print("  ");
          Serial.printf("[OTA] ESP32 heap size  %d \r\n", ESP.getHeapSize());
          Serial.println("  ");
        }

      }
      Serial.println("ota_client disconnected....");
    }
    OTA_4G_printPercent(readLength, contentLength);
  }

  timeElapsed = millis() - timeElapsed;
  Serial.println();
  file.close();

  // Shutdown

  ota_client.stop();
  Serial.println("Server disconnected");

#if TINY_GSM_USE_WIFI
  ota_modem.networkDisconnect();
  Serial.println(F("WiFi disconnected"));
#endif
#if TINY_GSM_USE_GPRS
  ota_modem.gprsDisconnect();
  Serial.println("GPRS disconnected");
#endif

  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");
  Serial.println(contentLength);
  Serial.print("Actually read:  ");
  Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x");
  Serial.println(crc.finalize(), HEX);
  Serial.print("Known CRC32:    0x");
  Serial.println(knownCRC32, HEX);
  Serial.print("Duration:       ");
  Serial.print(duration);
  Serial.println("s");
  // Do nothing forevermore
  // pu();
  // OTA_4G_pu(FFat, "/update.bin");
  OTA_4G_pu("/update.bin");
}



#if 0
void OTA_4G_setup_4G_OTA_get(void)
{
  // modem.gprsConnect(apn, gprsUser, gprsPass);
  // APN = getAPN();

  // strcpy(g_apn,APN.c_str());
  // setup4G();
  OTA_4G_setup4G();
  Serial.print("Waiting for network...");
  if (!ota_modem.waitForNetwork())
  {
    Serial.println(" fail");

#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isNetworkConnected())
  {
    Serial.println("Network connected");
  }
#if TINY_GSM_USE_GPRS | 1
  // GPRS connection parameters are usually set after network registration
  Serial.print(F("Connecting to "));
  Serial.print(g_apn);
  if (!ota_modem.gprsConnect(g_apn, gprsUser, gprsPass))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isGprsConnected())
  {
    Serial.println("GPRS connected");
  }
#endif
  Serial.print("Connecting to ");

  Serial.print(server);

  // strcpy(&fota_host[0], host_fota.c_str());
  // strcpy(&fota_path[0], path_fota.c_str());
  // fota_port = port_fota;

  // Serial.print(fota_host);

  if (!ota_client.connect(server, port))
    // if (!ota_client.connect(fota_host, fota_port))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  // ota_client.print(String("POST ") + resource + " HTTP/1.0\r\n");
  ota_client.print(String("GET ") + resource + " HTTP/1.1\r\n");
  ota_client.print(String("Host: ") + server + "\r\n");

  // dynamic update
  //  ota_client.print(String("GET ") + fota_path + " HTTP/1.1\r\n");
  //  ota_client.print(String("Host: ") + fota_host + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(WiFi.macAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-AP-MAC: ") + String(WiFi.softAPmacAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-sketch-md5: ") + String(ESP.getSketchMD5()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getSdkVersion()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getFreeSketchSpace()) + "\r\n");

  // ota_client.print(String("x-ESP32-firmware-version: ") + VERSION + "\r\n");
  // ota_client.print(String("x-ESP32-device-id: ") + "evse_001" + "\r\n");

  ota_client.print(String("x-ESP32-device-test-id: ") + DEVICE_ID + "\r\n");
  ota_client.print(String("x-ESP32-firmware-version: ") + EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION + "\r\n");

  ota_client.print("Connection: close\r\n\r\n");

  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();
  Serial.println(F("Waiting for response header"));

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the ota_client, we need to exit.

  const uint32_t ota_clientReadTimeout = 600000;
  uint32_t ota_clientReadStartTime = millis();
  String headerBuffer;
  bool finishedHeader = false;
  uint32_t contentLength = 0;

  while (!finishedHeader)
  {
    int nlPos;

    if (ota_client.available())
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        char c = ota_client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        if (c < 16)
          Serial.print('0');
        Serial.print(c, HEX);
        Serial.print(' ');
        /*if (isprint(c))
          Serial.print(reinterpret_cast<char> c);
          else
          Serial.print('*');*/
        Serial.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0)
          break;
      }
    }
    else
    {
      if (millis() - ota_clientReadStartTime > ota_clientReadTimeout)
      {
        // Time-out waiting for data from ota_client
        Serial.println(F(">>> ota_client Timeout !"));
        break;
      }
    }
    // See if we have a new line.
    nlPos = headerBuffer.indexOf(F("\r\n"));

    if (nlPos > 0)
    {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(F("content-length:")))
      {
        contentLength = headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        // Serial.print(F("Got Content Length: "));  // uncomment for
        // Serial.println(contentLength);            // confirmation
      }

      headerBuffer.remove(0, nlPos + 2); // remove the line
    }
    else if (nlPos == 0)
    {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line),
      // we are done with the header.
      finishedHeader = true;
    }
  }

  // The two cases which are not managed properly are as follows:
  // 1. The ota_client doesn't provide data quickly enough to keep up with this
  // loop.
  // 2. If the ota_client data is segmented in the middle of the 'Content-Length: '
  // header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32 crc;
  // File file = FFat.open("/update.bin", FILE_APPEND);
  // if (finishedHeader && contentLength == knownFileSize) {
  if (finishedHeader)
  {

    Serial.println(F("Reading response data"));
    ota_clientReadStartTime = millis();

    String get_response = "";
    // printPercent(readLength, contentLength);
    while (readLength < contentLength && ota_client.connected() &&
      millis() - ota_clientReadStartTime < ota_clientReadTimeout)
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        // uint8_t c = ota_client.read();
        char c = ota_client.read();
        if (c == 'f')
        {
          Serial.print("no OTA update");
          // gu8_OTA_update_flag = 3;
        }
        if (c == 't')
        {
          Serial.print("OTA update available");
          // gu8_OTA_update_flag = 2;
        }
        // Serial.print(c);
        get_response += c;
        // Serial.print(reinterpret_cast<char>c);  // Uncomment this to show
        // data
        // crc.update(c);
        readLength++;
      }
      Serial.println("\r\n" + get_response);

      if (get_response.equals("true") == true)
      {
        // gu8_OTA_update_flag = 2;
        // Serial.print("OTA update available");
      }
      else if (get_response.equals("false") == false)
      {
        // gu8_OTA_update_flag = 3;
        // Serial.print("no OTA update");
      }
      get_response = "";
      Serial.println("\r\nota_client disconnected....");
    }
    // printPercent(readLength, contentLength);
  }

  timeElapsed = millis() - timeElapsed;
  Serial.println();

  ota_client.stop();
  Serial.println(F("Server disconnected"));

#if TINY_GSM_USE_GPRS
  ota_modem.gprsDisconnect();
  Serial.println(F("GPRS disconnected"));
#endif

  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");
  Serial.println(contentLength);
  Serial.print("Actually read:  ");
  Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x");
  Serial.println(crc.finalize(), HEX);
  Serial.print("Known CRC32:    0x");
  Serial.println(knownCRC32, HEX);
  Serial.print("Duration:       ");
  Serial.print(duration);
  Serial.println("s");
}

#endif
///////////////////////////get_response///////////////////

//------------------------------------------ 4G OTA ----------------------------------------//

#if 0
void OTA_4G_setup_4G_OTA(void)
{
  // strcpy(g_apn,APN.c_str());
  OTA_4G_setup4G();
  Serial.print("Waiting for network...");
  if (!ota_modem.waitForNetwork())
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isNetworkConnected())
  {
    Serial.println("Network connected");
  }
#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
  Serial.print("Connecting to ");
  Serial.print(g_apn);
  if (!ota_modem.gprsConnect(g_apn, gprsUser, gprsPass))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");

  if (ota_modem.isGprsConnected())
  {
    Serial.println("GPRS connected");
  }
#endif
  Serial.print("Connecting to ");
  Serial.print(server);
  // Serial.print(fota_host);

  if (!ota_client.connect(server, port))
    // if (!ota_client.connect(fota_host, fota_port))
  {
    Serial.println(" fail");
#if FREE_RTOS_THREAD
    vTaskDelay(10000 / portTICK_PERIOD_MS);
#else
    delay(10000);
#endif
    return;
  }
  Serial.println(" success");
  // int sketch_md5=ESP.getSketchMD5();
  // Serial.println("sketch_md5 =" + String(sketch_md5));
  //  Make a HTTP GET request:

  ota_client.print(String("POST ") + resource + " HTTP/1.1\r\n");
  ota_client.print(String("Host: ") + server + "\r\n");
  // ota_client.print(String("POST ") + fota_path + " HTTP/1.1\r\n");
  // ota_client.print(String("Host: ") + fota_host + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(WiFi.macAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-AP-MAC: ") + String(WiFi.softAPmacAddress()) + "\r\n");
  ota_client.print(String("x-ESP32-sketch-md5: ") + String(ESP.getSketchMD5()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getSdkVersion()) + "\r\n");
  ota_client.print(String("x-ESP32-STA-MAC: ") + String(ESP.getFreeSketchSpace()) + "\r\n");

  // ota_client.print(String("x-ESP32-firmware-version: ") + VERSION + "\r\n");
  // ota_client.print(String("x-ESP32-device-id: ") + "evse_001" + "\r\n");
  ota_client.print(String("x-ESP32-device-test-id: ") + DEVICE_ID + "\r\n");
  ota_client.print(String("x-ESP32-firmware-version: ") + EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION + "\r\n");

  ota_client.print("Connection: close\r\n\r\n");

  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();
  Serial.println(F("Waiting for response header"));

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the ota_client, we need to exit.

  const uint32_t ota_clientReadTimeout = 600000;
  uint32_t ota_clientReadStartTime = millis();
  String headerBuffer;
  bool finishedHeader = false;
  uint32_t contentLength = 0;

  while (!finishedHeader)
  {
    int nlPos;

    if (ota_client.available())
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        char c = ota_client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        if (c < 16)
          Serial.print('0');
        Serial.print(c, HEX);
        Serial.print(' ');
        /*if (isprint(c))
          Serial.print(reinterpret_cast<char> c);
          else
          Serial.print('*');*/
        Serial.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0)
          break;
      }
    }
    else
    {
      if (millis() - ota_clientReadStartTime > ota_clientReadTimeout)
      {
        // Time-out waiting for data from ota_client
        Serial.println(F(">>> ota_client Timeout !"));
        break;
      }
    }
    // See if we have a new line.
    nlPos = headerBuffer.indexOf(F("\r\n"));

    if (nlPos > 0)
    {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(F("content-length:")))
      {
        contentLength = headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        Serial.print("Got Content Length: "); // uncomment for
        Serial.println(contentLength);        // confirmation
        // if(contentLength <= 0)
        // {
        //   // Serial.print(F("Got Content Length: "));  // comment at 06092023_170900
        //   // Serial.println(contentLength);
        //   contentLength = 1;
        // }
      }

      headerBuffer.remove(0, nlPos + 2); // remove the line
    }
    else if (nlPos == 0)
    {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line),
      // we are done with the header.
      finishedHeader = true;
    }
  }

  // The two cases which are not managed properly are as follows:
  // 1. The ota_client doesn't provide data quickly enough to keep up with this
  // loop.
  // 2. If the ota_client data is segmented in the middle of the 'Content-Length: '
  // header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32 crc;

#if 1
  File fs = FFat.open("/update.bin", FILE_APPEND);
  if (FFat.remove("/update.bin"))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
#endif

  // pinMode(WATCH_DOG_PIN, OUTPUT);
  // gu32reset_watch_dog_timer_count = 0;

  File file = FFat.open("/update.bin", FILE_APPEND);
  // if (finishedHeader && contentLength == knownFileSize) {
  if (finishedHeader)
  {

    Serial.println(F("Reading response data"));
    ota_clientReadStartTime = millis();

    OTA_4G_printPercent(readLength, contentLength);
    while (readLength < contentLength && ota_client.connected() &&
      millis() - ota_clientReadStartTime < ota_clientReadTimeout)
    {
      ota_clientReadStartTime = millis();
      while (ota_client.available())
      {
        uint8_t c = ota_client.read();
        // char c = ota_client.read();
        // Serial.print(c);
        if (!file.write(c))
        {
          Serial.println("not Appending file");
        }

        // Serial.print(reinterpret_cast<char>c);  // Uncomment this to show
        // data
        crc.update(c);
        readLength++;
        if (readLength % (contentLength / 100) == 0)
        {
          OTA_4G_printPercent(readLength, contentLength);
          Serial.print("  ");
          Serial.printf("[OTA] ESP32 heap size  %d \r\n", ESP.getHeapSize());
          Serial.println("  ");
        }

        // gu32reset_watch_dog_timer_count++;
        // if (gu32reset_watch_dog_timer_count > WATCH_DOG_RESET_COUNT + WATCH_DOG_RESET_INTERVAL)
        // {
        //   gu32reset_watch_dog_timer_count = 0;
        //   // ptr_watch_dog->evse_watch_dog_off();
        //   digitalWrite(WATCH_DOG_PIN, LOW);
        // }
        // else if (gu32reset_watch_dog_timer_count > WATCH_DOG_RESET_COUNT)
        // {
        //   // gu32reset_watch_dog_timer_count=0;
        //   // ptr_watch_dog->evse_watch_dog_on();
        //   digitalWrite(WATCH_DOG_PIN, HIGH);
        // }
      }
      Serial.println("ota_client disconnected....");
    }
    OTA_4G_printPercent(readLength, contentLength);
  }

  timeElapsed = millis() - timeElapsed;
  Serial.println();
  file.close();

  // Shutdown

  ota_client.stop();
  Serial.println(F("Server disconnected"));

#if TINY_GSM_USE_WIFI
  ota_modem.networkDisconnect();
  Serial.println(F("WiFi disconnected"));
#endif
#if TINY_GSM_USE_GPRS
  ota_modem.gprsDisconnect();
  Serial.println(F("GPRS disconnected"));
#endif

  float duration = float(timeElapsed) / 1000;

  Serial.println();
  Serial.print("Content-Length: ");
  Serial.println(contentLength);
  Serial.print("Actually read:  ");
  Serial.println(readLength);
  Serial.print("Calc. CRC32:    0x");
  Serial.println(crc.finalize(), HEX);
  Serial.print("Known CRC32:    0x");
  Serial.println(knownCRC32, HEX);
  Serial.print("Duration:       ");
  Serial.print(duration);
  Serial.println("s");
  // Do nothing forevermore
  // pu();
  // OTA_4G_pu(FFat, "/update.bin");
  OTA_4G_pu("/update.bin");
}
#endif

/////////////////////SETUP 4G////////////////////////////////////////////

#define GSM_RXD2 16
#define GSM_TXD2 17

#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

void OTA_4G_setup4G(void)
{
  SerialAT.begin(115200, SERIAL_8N1, GSM_RXD2, GSM_TXD2);
  Serial.println("[CustomSIM7672] Starting 4G Setup");
  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  SerialAT.println("AT+CMGD=1,4");
  Serial.println("AT+CMGD=1,4");
  OTA_4G_waitForResp(20);
  Serial.println("[CustomSIM7672] ota_modem restart...");
  ota_modem.restart();
  delay(200);
  Serial.println("[CustomSIM7672] Initializing ota_modem...");

  String ota_modemInfo = ota_modem.getModemInfo();
  Serial.print("[CustomSIM7672] ota_modem Info: ");
  Serial.println(ota_modemInfo);
  ota_modem.gprsConnect(APN.c_str(), gprsUser, gprsPass);
  Serial.println("[CustomSIM7672] Waiting for network...");
  if (!ota_modem.waitForNetwork())
  {
    Serial.println("[CustomSIM7672] fail");
    // delay(200);
#if FREE_RTOS_THREAD
    vTaskDelay(200 / portTICK_PERIOD_MS);
#else
    delay(200);
#endif
    return;
  }
  Serial.println("[CustomSIM7672] success");
  if (ota_modem.isNetworkConnected())
  {
    Serial.println("[CustomSIM7672] Network connected");
  }
  Serial.print("[CustomSIM7672] Connecting to APN :  ");
  Serial.print(APN.c_str());
  if (!ota_modem.gprsConnect(APN.c_str(), gprsUser, gprsPass))
  {
    Serial.println("[CustomSIM7672] fail");
    // delay(200);
#if FREE_RTOS_THREAD
    vTaskDelay(200 / portTICK_PERIOD_MS);
#else
    delay(200);
#endif
    // gsm_net = false;
    return;
  }
  Serial.println("[CustomSIM7672] success");
  if (ota_modem.isGprsConnected())
  {
    Serial.println("[CustomSIM7672] 4G connected");
    // gsm_net = true;
  }
  int csq = ota_modem.getSignalQuality();
  Serial.println("Signal quality: " + String(csq));
  // delay(1);
#if FREE_RTOS_THREAD
  vTaskDelay(1 / portTICK_PERIOD_MS);
#else
  delay(1);
#endif
}
/////////////////////SETUP 4G////////////////////////////////////////////

/*************************fatfs***********/

/*waitForResp*/

uint8_t OTA_4G_waitForResp(uint8_t timeout)
{
  const char* crtResp = "+HTTPACTION: 1,200"; // Success
  const char* Resp400 = "+HTTPACTION: 1,4";   // Conf Error
  const char* Resp500 = "+HTTPACTION: 1,5";   // Internal Server Error
  const char* Resp700 = "+HTTPACTION: 1,7";   // Internal Server Error
  const char* errResp = "+HTTP_NONET_EVENT";
  const char* errResp1 = "ERROR";
  const char* okResp = "OK";
  const char* DOWNResp = "DOWNLOAD";
  const char* clkResp = "+CCLK: ";      // Success
  const char* pdndeactResp = "+CGEV: "; // pdn deact
  const char* mqttconnectResp = "+CMQTTCONNECT: 0,0";
  const char* mqttcheckconnResp = "+CMQTTCONNECT: 0,\"tcp://34.134.133.145:1883\",60,1";
  const char* mqttdisconnResp = "+CMQTTDISC: 0,0";

  uint8_t len1 = strlen(crtResp);
  uint8_t len2 = strlen(Resp400);
  uint8_t len3 = strlen(errResp);
  uint8_t len4 = strlen(Resp700);
  uint8_t len5 = strlen(errResp1);
  uint8_t len6 = strlen(okResp);
  uint8_t len7 = strlen(DOWNResp);
  uint8_t len8 = strlen(clkResp);
  uint8_t len11 = strlen(pdndeactResp);
  uint8_t len12 = strlen(mqttconnectResp);
  uint8_t len13 = strlen(mqttcheckconnResp);
  uint8_t len14 = strlen(mqttdisconnResp);

  uint8_t sum1 = 0;
  uint8_t sum2 = 0;
  uint8_t sum3 = 0;
  uint8_t sum4 = 0;
  uint8_t sum5 = 0;
  uint8_t sum6 = 0;
  uint8_t sum7 = 0;
  uint8_t sum8 = 0;
  uint8_t sum11 = 0;
  uint8_t sum12 = 0;
  uint8_t sum13 = 0;
  uint8_t sum14 = 0;
  unsigned long timerStart, timerEnd;
  timerStart = millis();
  while (1)
  {
    timerEnd = millis();
    if (timerEnd - timerStart > 1000 * timeout)
    {
      // gsm_net = false;
      return -1; // Timed out
    }
    if (SerialAT.available())
    {
      char c = SerialAT.read();
      Serial.print(c);
      sum1 = (c == crtResp[sum1]) ? sum1 + 1 : 0;
      sum2 = (c == Resp400[sum2]) ? sum2 + 1 : 0;
      sum3 = (c == errResp[sum3]) ? sum3 + 1 : 0;
      sum4 = (c == Resp700[sum4]) ? sum4 + 1 : 0;
      sum5 = (c == errResp1[sum5]) ? sum5 + 1 : 0;
      sum6 = (c == okResp[sum6]) ? sum6 + 1 : 0;
      sum7 = (c == DOWNResp[sum7]) ? sum7 + 1 : 0;
      sum8 = (c == clkResp[sum8]) ? sum8 + 1 : 0;
      sum11 = (c == pdndeactResp[sum11]) ? sum11 + 1 : 0;
      sum12 = (c == mqttconnectResp[sum12]) ? sum12 + 1 : 0;
      sum13 = (c == mqttcheckconnResp[sum13]) ? sum13 + 1 : 0;
      sum14 = (c == mqttdisconnResp[sum14]) ? sum14 + 1 : 0;

      if (sum1 == len1)
      {
        // gsm_net = true;
        Serial.println("Success!");
        return 1;
      }
      else if (sum2 == len2)
      {
        // gsm_net = true;
        Serial.println("400 Error!");
        return 0;
      }
      else if (sum3 == len3)
      {
        Serial.println("NONET Error!");
        // gsm_net = false;
        return 0;
      }
      else if (sum4 == len4)
      {
        Serial.println("700 No internet!");
        // gsm_net = false;
        return 0;
      }
      else if (sum5 == len5)
      {
        return 0;
      }
      else if (sum6 == len6)
      {
        Serial.println("AT_OK");
        return 0;
      }
      else if (sum7 == len7)
      {
        return 0;
      }
      else if (sum8 == len8)
      {
        Serial.println("OK");
#if 0
        Serial.print(F("Timestamp : "));
        //Serial.println(timestamp);
        for (int i = 0; i < strlen(timestamp); i++)
        {
          Serial.print(i);
          Serial.print(":");
          Serial.println(timestamp[i]);
        }
        //timeEpoch();
#endif
        return 0;
      }
      else if (sum11 == len11)
      {
        /*
          A7600 Series_ AT Command Manual

          +CGEV: ME PDN DEACT 1
           The mobile termination has deactivated a context.
           The context represents a PDN connection in LTE or a Primary PDP context in GSM/UMTS.
           The <cid> for this context is provided to the TE.
           The format of the parameter <cid> NOT E is found in command +CGDCONT.
        */
        Serial.println("PDN DEACTED ...!");
        // gsm_net = false;
        return 0;
      }
      else if (sum12 == len12)
      {
        Serial.println("Device connected to server using mqtt");
        return 0;
      }
      else if (sum13 == len13)
      {
        Serial.println("Already connected");
        return 0;
      }
      else if (sum14 == len14)
      {
        Serial.println("mqtt disconnected");
        return 0;
      }
    }
  }
  while (SerialAT.available())
  {
    SerialAT.read();
  }
  return 0;
}
/*waitForResp*/

/*print percentage */

void OTA_4G_printPercent(uint32_t readLength, uint32_t contentLength)
{
  // If we know the total length
  if (contentLength != (uint32_t)-1)
  {
    Serial.print("\r ");
    Serial.print((100.0 * readLength) / contentLength);
    Serial.print('%');
  }
  else
  {
    Serial.println(readLength);
  }
}
/*print percentage */

void OTA_4G_pu(const char* path)
{
  // FS fs;
  Serial.println("***Starting OTA update***");
  File updateBin = FFat.open(path);

  if (updateBin)
  {
    if (updateBin.isDirectory())
    {
      Serial.println("Directory error");
      updateBin.close();
      return;
    }

    size_t updateSize = updateBin.size();

    if (Update.begin(updateSize))
    {
      size_t written = Update.writeStream(updateBin);
      if (written == updateSize)
      {
        Serial.println("Writes : " + String(written) + " successfully");
      }
      else
      {
        Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end())
      {
        Serial.println("OTA finished!");
        if (Update.isFinished())
        {
          Serial.println("Restart ESP device!");
          // esp_deep_sleep(1000 * 1000);

          ESP.restart();
        }
        else
        {
          Serial.println("OTA not finished yet");
        }
      }
      else
      {
        Serial.println("Error occured #: " + String(Update.getError()));
        // esp_deep_sleep(1000 * 1000);
        ESP.restart();
      }
    }
    else
    {
      Serial.println("Cannot begin update");
    }
  }
  updateBin.close();
}

#endif