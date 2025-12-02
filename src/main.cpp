#include <settings.h>
#include <bme280.h>
#include <lcd.h>
#include <ntc.h>
#include <QuickPID.h>
#include <settings.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Update.h>

#ifndef MAJOR
#define MAJOR 999
#endif
#ifndef MINOR
#define MINOR 999
#endif
#ifndef PATCH
#define PATCH 999
#endif

#define SHOW_TOP_MS 1000
#define SHOW_MIDLE_MS 3000
#define SHOW_BOTTOM_MS 1000

#define BUTTON_UP 2
#define BUTTON_DOWN 5
#define BUTTON_PLUS 33

#define HEAT_PIN 16
#define HEAT_CH 0
#define HEAT_FREQ 5000
#define HEAT_RES 10

#define FAN_PIN 17
#define FAN_CH 1
#define FAN_FREQ 1000
#define FAN_RES 10


enum menuValueType
{
  _inc_dec,
  _bool,
  _hours,
  _minuts,
  _save_exit,
  _exit_one_save,
};

struct MenuItem
{
  const char *text;
  const char *value;
  int id;
};

MenuItem menuItems[] = {
    {"Set Temp", "", 1},
    {"Enable Timer", "", 2},
    {"Hours", "", 3},
    {"Minutes", "", 4},
    {"Heater", "", 5},
    {"Fan", "", 6},
    {"WiFi", "", 7},
    {"Save & Exit", "", 8},
    {"Exit", "", 9}};

static const int MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);
char valueBufs[MENU_COUNT][16];

float set_point, temp_input, heat_output;
float Kp = 2, Ki = 5, Kd = 1;

Settings settings;
QuickPID myPID(&temp_input, &heat_output, &set_point);
WebServer server(80);
String hostname;
uint32_t last_heat_run = 0;
bool flag_ap = false;

const char *updatePage =
    "<!DOCTYPE html><html><body>"
    
    "<div style=\"background-color: lightgreen; width: 400px; margin: 0 auto;\" >"
    "<h2>DryBox OTA Update</h2>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='firmware'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "</div>"
    "</body></html>";

void handleRootAp()
{
  server.send(200, "text/html", updatePage);
}

void handleUpdateUpload()
{
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START)
  {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
    {
      Update.printError(Serial);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (Update.end(true))
    {
      Serial.printf("Update Success: %u bytes\n", upload.totalSize);
    }
    else
    {
      Update.printError(Serial);
    }
  }
}

void handleUpdateFinished()
{
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", Update.hasError() ? "Update failed!" : "Update OK, rebooting...");
  delay(1000);
  ESP.restart();
}

void startAccessPoint()
{
  Serial.println("Starting AP mode...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WiFi.getHostname(), "12345678");

  Serial.println("AP IP: " + WiFi.softAPIP().toString());

  server.on("/", HTTP_GET, handleRootAp);
  server.on("/update", HTTP_POST, handleUpdateFinished, handleUpdateUpload);

  server.begin();
}

void updateMenuValues();

void setHostName()
{
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String mac_last4 = mac.substring(mac.length() - 6);
  hostname = "DryBox-" + mac_last4;

  WiFi.setHostname(hostname.c_str());
}

void setup()
{
  Serial.begin(9600);
  Serial.printf("Start PLC1 V%d.%d.%d\n", MAJOR, MINOR, PATCH);

  settings = Settings();
  InitBME();
  InitLCD();
  updateMenuValues();
  showStatic(settings.getTemp(), settings.getHeaterState(), settings.getFanState(), settings.getWiFiState());
  showMidle(0.0, 0.0, false);
  pinMode(BUTTON_PLUS, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  ledcSetup(HEAT_CH, HEAT_FREQ, HEAT_RES);
  ledcAttachPin(HEAT_PIN, HEAT_CH);
  ledcSetup(FAN_CH, FAN_FREQ, FAN_RES);
  ledcAttachPin(FAN_PIN, FAN_CH);

  myPID.SetTunings(Kp, Ki, Kd);
  myPID.SetMode(1);
  myPID.SetOutputLimits(0, 1023);
  set_point = settings.getTemp();
  setHostName();

  if (!digitalRead(BUTTON_UP) && !digitalRead(BUTTON_DOWN))
  {
    startAccessPoint();
    flag_ap = true;
  }
}

void changeSettings()
{
  set_point = settings.getTemp();
  showStatic(settings.getTemp(), settings.getHeaterState(), settings.getFanState(), true);
}

void lcdTask()
{
  static bool isTemp = true;

  static uint32_t timer_TOP = 0;
  if ((millis() - timer_TOP) > SHOW_TOP_MS)
  {
    timer_TOP = millis();
  }

  static uint32_t timer_midle = 0;
  if ((millis() - timer_midle) > SHOW_MIDLE_MS)
  {
    timer_midle = millis();
    showMidle(isTemp ? getBMETemperatuere() : getBMEHumidity(), getHeaterTemperature(), getBMEError());
    showIcon(isTemp);
    isTemp = !isTemp;
  }

  static uint32_t timer_bottom = 0;
  if ((millis() - timer_bottom) > SHOW_BOTTOM_MS)
  {
    timer_bottom = millis();
    showBottom(settings.getTimerTime(), settings.getTimerSate());
  }
}

void pidTask()
{

  static bool pre_heater_state = false;
  if (settings.getHeaterState())
  {
    double _temp = getHeaterTemperature();
    if (_temp < 0)
    {
      _temp = 500;
    }
    temp_input = _temp + 3;
    myPID.Compute();
    ledcWrite(HEAT_CH, heat_output);
  }
  else
  {
    ledcWrite(HEAT_CH, 0);
  }

  if (heat_output > 0)
  {
    last_heat_run = millis();
  }

  if (pre_heater_state != (heat_output > 0 ? true : false))
  {
    pre_heater_state = heat_output > 0 ? true : false;
    showHeaterStatus(pre_heater_state);
  }
}

void fanTask()
{

  static bool pre_fan_state;
  static int fan_output = 0;
  if (settings.getFanState())
  {
    if (((int)getHeaterTemperature() > settings.getTemp()) || ((millis() - last_heat_run) < 10000))
    {

      if (fan_output < 20)
      {

        fan_output++;
      }
      else
      {
        fan_output = 1023;
      }
    }
    else
    {
      fan_output = 0;
    }

    ledcWrite(FAN_CH, fan_output);
  }
  else
  {
    ledcWrite(FAN_CH, 0);
  }

  if ((((int)getHeaterTemperature()) < 0) || (((int)getHeaterTemperature()) > 100))
  {
    ledcWrite(FAN_CH, 1023);
  }

  if (pre_fan_state != (fan_output > 0 ? true : false))
  {
    pre_fan_state = fan_output > 0 ? true : false;
    showFanStatus(pre_fan_state);
  }
}

void updateMenuValues()
{
  sprintf(valueBufs[0], "%dC", int(settings.getTemp()));
  menuItems[0].value = valueBufs[0];

  sprintf(valueBufs[1], "%s", settings.getTimerSate() ? "ON" : "OFF");
  menuItems[1].value = valueBufs[1];

  sprintf(valueBufs[2], "%02d", 1);
  menuItems[2].value = valueBufs[2];

  sprintf(valueBufs[3], "%02d", 15);
  menuItems[3].value = valueBufs[3];

  sprintf(valueBufs[4], "%s", settings.getHeaterState() ? "ON" : "OFF");
  menuItems[4].value = valueBufs[4];

  sprintf(valueBufs[5], "%s", settings.getFanState() ? "ON" : "OFF");
  menuItems[5].value = valueBufs[5];

  sprintf(valueBufs[6], "%s", settings.getWiFiState() ? "ON" : "OFF");
  menuItems[6].value = valueBufs[6];

  valueBufs[7][0] = '\0';
  menuItems[7].value = valueBufs[7];

  valueBufs[8][0] = '\0';
  menuItems[8].value = valueBufs[8];
}

void changeMenuValue(int *menuState, bool inc)
{
  Serial.println(*menuState);
  if (*menuState < 1 || *menuState > MENU_COUNT)
  {
    drawMenuLine("", "", true);
    return;
  }

  int delta = inc ? 1 : -1;
  MenuItem &item = menuItems[*menuState - 1];

  switch (*menuState)
  {
  case 1:
  {
    int temp = int(settings.getTemp()) + delta;
    if (temp < SET_TEMP_MIN)
      temp = SET_TEMP_MIN;
    if (temp > SET_TEMP_MAX)
      temp = SET_TEMP_MAX;
    settings.setTemp(temp);
    break;
  }

  case 2:
  {
    settings.setTimerState(!settings.getTimerSate());
    break;
  }

  case 3:
  {
    static int hours = 1;
    hours += delta;
    if (hours < 0)
      hours = 23;
    if (hours > 23)
      hours = 0;
    sprintf(valueBufs[2], "%02d", hours);
    item.value = valueBufs[2];
    break;
  }

  case 4:
  {
    static int minutes = 15;
    minutes += delta * 5;
    if (minutes < 0)
      minutes = 55;
    if (minutes > 55)
      minutes = 0;
    sprintf(valueBufs[3], "%02d", minutes);
    item.value = valueBufs[3];
    break;
  }

  case 5:
  {
    bool state = !settings.getHeaterState();
    settings.setHeaterState(state);
    sprintf(valueBufs[4], "%s", state ? "ON" : "OFF");
    item.value = valueBufs[4];
    break;
  }

  case 6:
  {
    bool state = !settings.getFanState();
    settings.setFanState(state);
    sprintf(valueBufs[5], "%s", state ? "ON" : "OFF");
    item.value = valueBufs[5];
    break;
  }

  case 7:
  {
    Serial.println(settings.getWiFiState());
    bool state = !settings.getWiFiState();
    settings.setWiFiState(state);
    Serial.println(settings.getWiFiState());
    sprintf(valueBufs[6], "%s", state ? "ON" : "OFF");
    item.value = valueBufs[6];
    break;
  }

  case 8:
    *menuState = 0;
    drawMenuLine("", "", true);
    settings.saveSettings();
    updateMenuValues();
    changeSettings();

    return;
  case 9:
    settings.loadSettings();
    updateMenuValues();
    *menuState = 0;
    drawMenuLine("", "", true);
    break;
  }
  updateMenuValues();
  drawMenuLine(menuItems[*menuState - 1].text, menuItems[*menuState - 1].value, false);
}

void lcdMenuTask()
{
  static int menuState = 0;

  static uint32_t timer = 0;

  if ((millis() - timer) > 100)
  {
    timer = millis();
  }
  else
  {
    return;
  }

  if (!digitalRead(BUTTON_UP))
  {
    changeMenuValue(&menuState, true);
    Serial.println("UP");
  }
  else if (!digitalRead(BUTTON_DOWN))
  {
    changeMenuValue(&menuState, false);
    Serial.println("DOWN");
  }
  else if (!digitalRead(BUTTON_PLUS))
  {
    if (menuState == MENU_COUNT)
    {
      menuState = 1;
    }
    else
    {
      menuState++;
    }

    drawMenuLine(menuItems[menuState - 1].text, menuItems[menuState - 1].value, false);

    Serial.println("NEXt");
    Serial.println(menuState);
  }
}

float round1(float val)
{
  return round(val * 10.0) / 10.0;
}

void handleRoot()
{
  StaticJsonDocument<512> json;

  json["name"] = hostname;
  json["version"] = "%d.%d.%d", MAJOR, MINOR, PATCH;
  json["box_temperature"] = round1(getBMETemperatuere());
  json["box_humidity"] = round1(getBMEHumidity());
  json["heater_temperature"] = round1(getHeaterTemperature());
  json["heater_state"] = heat_output > 0 ? true :false;
  json["fan_state"] = 1;
  json["timer_state"] = 1;
  json["timer_remaining"] = (settings.getTimerTime() - millis()) / 1000;
  json["run_time"] = millis() / 1000;

  JsonObject setting = json.createNestedObject("settings");
  setting["target_temperature"] = round1(settings.getTemp());
  setting["heater_enabled"] = settings.getHeaterState();
  setting["fan_enabled"] = settings.getFanState();
  setting["timer_enabled"] = settings.getTimerSate();

  String response;
  serializeJson(json, response);

  server.send(200, "application/json", response);
}

void wifiTask()
{
  static bool wf_state = false;
  if (settings.getWiFiState())
  {
    if (!wf_state)
    {
      WiFi.begin(settings.getWiFiSSID(), settings.getWiFiPass());
      wf_state = true;
    }
  }
  else
  {
    if (wf_state)
    {
      WiFi.disconnect(false);
      wf_state = false;
    }
  }

  static bool connectet_weiv = false;
  static bool disconnectet_weiv = false;
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!connectet_weiv)
    {
      connectet_weiv = true;
      disconnectet_weiv = false;
      showWIFIStatus(true);
      if (!MDNS.begin(hostname.c_str()))
      {
        Serial.println("Error setting up MDNS responder!");
      }
      else
      {
        Serial.println("mDNS responder started");
      }
      server.on("/", handleRoot);
      server.on("/upd", handleRootAp);
      server.on("/update", HTTP_POST, handleUpdateFinished, handleUpdateUpload);
      server.begin();
    }
  }
  else
  {
    if (!disconnectet_weiv)
    {
      disconnectet_weiv = true;
      connectet_weiv = false;
      showWIFIStatus(false);
    }
  }
}

void loop()
{
  server.handleClient();

  wifiTask();
  if (flag_ap)
  {
    server.handleClient();
  }
  else
  {
    pidTask();
    fanTask();

    BMETask();
    NTCTask();
    lcdTask();
    lcdMenuTask();
  }
  // static uint32_t timer = 0;
  // if ((millis() - timer) > 1000)
  // {
  //   timer = millis();
  //   printf("%d\n", (int)getHeaterTemperature());
  // }
  //     ESP.restart();
}