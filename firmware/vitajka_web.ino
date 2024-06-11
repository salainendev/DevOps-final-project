#include <BluetoothSerial.h>
#include <Ticker.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define HR1 7
#define HR2 2


#define BLUETOOTH_PASS "1641"
#define WIFI_KEY_INIT 120
#define WIFI_SSID_PASS_DL 20
#define WIDELENO 42                   //2 * WIFI_SSID_PASS_DL + 2
#define TOCHKA_P 22                       //WIFI_SSID_PASS_DL + 2  


WiFiServer server(80);

uint8_t LED1pin = 2;

Ticker periodicTicker;
Ticker periodicTicker2;
Ticker onceTicker12;

BluetoothSerial ESP_BT;
volatile int cl = 0;

String ssid, pswd;
short wi_fi_eprom;
char str_s_p[WIFI_SSID_PASS_DL];

void callback_BT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    ESP_BT.println("Print Bluetooth password");
    cl = cl +1;
  }
  if(event == ESP_SPP_CLOSE_EVT){
    Serial.println("Client Disconnected");
  }
}


void Blink(){
    digitalWrite(LED1pin, !digitalRead(LED1pin));
}
volatile int tick1 = 0;
void Tick1(){
    tick1++;
}


String bt_read_string(){
  while (ESP_BT.available() == 0){

  }
  String incoming = ESP_BT.readString();
  incoming.replace("\n","");
  incoming.replace("\r","");
  Serial.print("Received:");
  Serial.println(incoming+"::"+incoming.length());
  return incoming;
}

void pass_ssid_connect(){
  ESP_BT.println("Print SSID");
  ssid = bt_read_string();
  ESP_BT.println("Print password");
  pswd = bt_read_string();
}

int err_WI_FI = 0;
bool wi_fi_connecting(){
    WiFi.disconnect();
    onceTicker12.attach_ms(1000, Tick1);
    while(Tick1 == 0){}
    WiFi.begin(ssid.c_str(), pswd.c_str());
    int sch = 0;
    auto ara = WiFi.waitForConnectResult();
    while (ara == WL_IDLE_STATUS){
      Serial.println("Connecting to WiFI");
      ara = WiFi.waitForConnectResult();
    }
    while(sch < 10){
      Serial.println("Connecting to WiFI::" + ssid + "::" + pswd + "::Status::" + WiFi.status() + "::pop::"+ sch);
      periodicTicker2.detach();
      if(ara == WL_NO_SSID_AVAIL){
        err_WI_FI = 1;
        Serial.println("No SSID::" + ssid + "::");
        Serial.println("No SSID::" + ssid + "::");
        sch = 5;
      }
      if(ara == WL_CONNECT_FAILED){
        err_WI_FI = 2;
        Serial.println("CONNECT FAILED");
        sch = 5;
      }
      if(ara == WL_NO_SHIELD){
        err_WI_FI = 2;
        Serial.println("NO SHIELD");
        sch = 5;
      }
      if(ara == WL_CONNECTED ){
        Serial.println("CONNECTED");
        sch = 5;
        return true;
      }
      if(sch == 5){
        return false;
      }
      sch++;
    }
}

bool wi_fi_bl_init(){
  ESP_BT.register_callback(callback_BT);
  
  while(true){
    periodicTicker.attach_ms(200, Blink);
    tick1 = 0;
    Serial.println("WAIT CLIENT");
    onceTicker12.attach_ms(1000, Tick1);
    int tick1_pr = 0;
    display.clearDisplay();
    // Display Text
    display.setTextSize(HR2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Bluetooth");
    display.println("Wait: 30s");
    display.display();
    while(tick1 < 30){
      if(tick1 != tick1_pr){
        display.clearDisplay();
        // Display Text
        display.setTextSize(HR2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Bluetooth");
        display.println("Wait: "+String(30-tick1)+String("s"));
        display.display();
        tick1_pr = tick1;
      }
      if(cl != 0 ){
        onceTicker12.detach();
        display.clearDisplay();
        // Display Text
        display.setTextSize(HR2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Bluetooth");
        display.println("Connect...");
        display.display();
        tick1_pr = tick1;
        break;
      }
    }
    onceTicker12.detach();
    Serial.println("WAIT CLIENT END");
    if(cl == 0 and wi_fi_eprom == WIFI_KEY_INIT){
      Serial.println("CONNECT EPROM SSID PASSWORD");
      display.clearDisplay();
      // Display Text
      display.setTextSize(HR2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("WI-FI");
      display.println("Connect\nold");
      display.display();
      tick1_pr = tick1;
      bool ret2 = wi_fi_connecting();
      if(ret2 == 1){
        Serial.println("CONNECT EPROM SSID PASSWORD TRUE");
        ESP_BT.end();
        Serial.println("Bluetooth Device off");
        periodicTicker.detach();
        digitalWrite(LED1pin, 0);
        display.setTextSize(HR2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("WI-FI");
        display.println("Connected\nold");
        display.display();
        return true;
      }
      else{
        Serial.println("CONNECT EPROM SSID PASSWORD FALSE");
        display.clearDisplay();
        display.setTextSize(HR2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("WI-FI");
        display.println("Connect false");
        display.display();
        
        display.clearDisplay();
        display.setTextSize(HR2);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Bluetooth");
        display.println("Wait");
        display.display();
      }
    }
    String passwd_bl = BLUETOOTH_PASS;
    int pop = 0;
    int init = 0;
    while(true){
      
      if(passwd_bl ==  bt_read_string()){
        init = 1;
        Serial.println("True password");
        ESP_BT.println("True password");
        display.println("PASS YES");
        break;
      }
      else{
        ESP_BT.println("False password");
        Serial.println("False password");
        display.println("PASS FALLSE");
        pop++;
      }
      if(pop == 5){
        ESP_BT.println("Disconnect");
        Serial.println("Bluetooth Device off");
        ESP_BT.end();
        return false;
      }
    }
    if(init == 1){
      break;
    }
  }
  
  periodicTicker.attach_ms(1000, Blink);
  display.clearDisplay();
  display.setTextSize(HR2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("WI-FI");
  display.println("Connect");
  display.display();
  pass_ssid_connect();
  bool ret1 = wi_fi_connecting(); 
  while(ret1!= 1){
    if(err_WI_FI == 1){
      ESP_BT.println("No SSID::" + ssid);
    }
    else if(err_WI_FI == 2){
      ESP_BT.println("CONNECT FAILED");
    }
    pass_ssid_connect();
    ret1 = wi_fi_connecting(); 
  }
  //ESP_BT.println("CONNECTED");
  //ESP_BT.println("IP:");
  ESP_BT.println("IP"+WiFi.localIP().toString());
  delay(100);
  bt_read_string();
  ESP_BT.end();
  Serial.println("Bluetooth Device off");
  periodicTicker.detach();
  digitalWrite(LED1pin, 0);
  Serial.println("EPROM WRITE SSID PASSWORD");
  wi_fi_eprom = WIFI_KEY_INIT;
  EEPROM.put(0, wi_fi_eprom);
  strncpy(str_s_p, ssid.c_str(), ssid.length()*sizeof(ssid[0]));
  str_s_p[ssid.length()*sizeof(ssid[0])] = 0;
  Serial.print("SSID: ");
  Serial.println(str_s_p);
  EEPROM.put(2, str_s_p);
  strncpy(str_s_p, pswd.c_str(), pswd.length()*sizeof(pswd[0]));
  str_s_p[pswd.length()*sizeof(pswd[0])] = 0;
  Serial.print("PASSWORD: ");
  Serial.println(str_s_p);
  EEPROM.put(TOCHKA_P, str_s_p);
  EEPROM.commit();     // для esp8266/esp32
  display.clearDisplay();
  display.setTextSize(HR2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("WI-FI");
  display.println("Connected");
  display.display();
  return true;
}

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
volatile bool led_sost = 0;
volatile float dutyCycle = 25.5;

void Add_l(){
   if((dutyCycle + 25.5) <= 255 and led_sost){
    dutyCycle = dutyCycle + 25.5; 
    ledcWrite(ledChannel, dutyCycle);
   }
}

void Sub_l(){
    if((dutyCycle-25.5 > 25.5) and led_sost){
      dutyCycle = dutyCycle - 25.5; 
      ledcWrite(ledChannel, dutyCycle);
   }
}

void On_off_L(){
   if(led_sost){
      ledcWrite(ledChannel, 0);
   }
   else{
      ledcWrite(ledChannel, dutyCycle);
   }
   led_sost = !led_sost;
}
void On_L(){
   if(!led_sost){
      ledcWrite(ledChannel, dutyCycle);
      led_sost = !led_sost;
   }
}
void Off_L(){
   if(led_sost){
      ledcWrite(ledChannel, 0);
      led_sost = !led_sost;
   }
}


void pr_dis_pr(int proc){
  // Clear the buffer.
  display.clearDisplay();
  // Display Text
  display.setTextSize(HR1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(String(proc));
  display.println("%");
  display.display();
  //delay(2000);
  //display.clearDisplay();
}

// setting PWM properties 2
const int prChannel = 1;
volatile bool pr_sost = 0;
volatile float dutyCycle_pr = 25.5;

void Add_pr(){
   if((dutyCycle_pr + 25.5) <= 255 and pr_sost){
    dutyCycle_pr = dutyCycle_pr + 25.5; 
    ledcWrite(prChannel, dutyCycle_pr);
   }
}

void Sub_pr(){
    if((dutyCycle_pr-25.5 > 25.5) and pr_sost){
      dutyCycle_pr = dutyCycle_pr - 25.5; 
      ledcWrite(prChannel, dutyCycle_pr);
   }
}

void sdel_pr(int power){
  if(!pr_sost){
    On_off_pr();
  }
    if(power*(2.55) <= 255 and (power*2.55) >= 0){
      dutyCycle_pr = power*(2.55);
      ledcWrite(prChannel, dutyCycle_pr);
    }
   display_update();
}

void On_off_pr(){
   if(pr_sost){
      ledcWrite(prChannel, 0);
   }
   else{
      ledcWrite(prChannel, dutyCycle_pr);
   }
   pr_sost = !pr_sost;
   display_update();
}

void setup() {
  Serial.begin(115200);
  delay(100);
////////////////////////////////////////////////////////////////////////////
  Serial.begin(115200);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  
///////////////////////////////////////////////////////////////////////////
  EEPROM.begin(WIDELENO);
  WiFi.mode(WIFI_STA);
  Serial.println("EPROM INIT");
  
  pinMode (LED1pin, OUTPUT);
  pinMode (14, INPUT);  // on\off
  pinMode (23, INPUT);  //+
  pinMode (16, INPUT);  //-

  pinMode (17, INPUT);  // on\off
  pinMode (19, INPUT);  //+
  pinMode (18, INPUT);  //-
  
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(32, ledChannel);

  ledcSetup(prChannel, freq, resolution);
  ledcAttachPin(27, prChannel);
  
  Serial.println("PIN INIT");
  
  attachInterrupt(14, On_off_L, RISING);
  attachInterrupt(23, Add_l, RISING);
  attachInterrupt(16, Sub_l, RISING);

  attachInterrupt(17, On_off_pr, RISING);
  attachInterrupt(19, Add_pr, RISING);
  attachInterrupt(18, Sub_pr, RISING);
  
  Serial.println("PRERIVANIA INIT");
  Serial.println("Bluetooth Device on");
  ESP_BT.begin("Esp");
  EEPROM.get(0, wi_fi_eprom);
  Serial.println("EPROM READ INIT WI-FI");
  if(wi_fi_eprom == WIFI_KEY_INIT){
    Serial.println("EPROM READ SSID PASSWORD");
    EEPROM.get(2,str_s_p);
    ssid = str_s_p;
    Serial.println("SSID "+ssid);
    EEPROM.get(TOCHKA_P, str_s_p);
    pswd = str_s_p;
    Serial.println("PASSWORD "+pswd);
  }
  else{
    Serial.println("EPROM NOT INIT WI-FI");
    wi_fi_eprom = 0;
  }
  while(true){
    if(wi_fi_bl_init() == true){
      long sig = WiFi.RSSI();
      Serial.println(String("Connected to WiFi Signal: ") + String(sig) + String(" dBm"));
      Serial.print(String("IP: "));
      Serial.print(WiFi.localIP());
      break;
    }
    else{
      Serial.print("Connect error");
      Serial.println("Bluetooth Device on");
      ESP_BT.begin("Esp");
    }
  }
  server.begin();
}

short post = 0;

void display_update(){
  if(pr_sost){
    pr_dis_pr(dutyCycle_pr/2.55);
  }
  else{
    display.clearDisplay();
    // Display Text
    display.setTextSize(HR1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("OFF");
    display.display();
  }
}

void loop() {
  // слушаем входящие клиенты
  display_update();
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Новый клиент");

    bool currentLineIsBlank = true;
    while (client.connected()) {
      display_update();
      if (client.available()) {
        String ceee = client.readString();
        String c2 = ceee;
        Serial.print(c2);
        String c;
        String ob;
        String de;
        size_t pos1 = 0;
        while ((pos1 = ceee.indexOf('\n')) != std::string::npos) {
        c = ceee.substring(0, pos1);
        ceee.remove(0, pos1 + 1);
          if(c[0] == 'G' and c[1] == 'E'){
            size_t pos = 0;
            size_t endstr = 0;
            String token;
            String zapros;
            while ((pos = c.indexOf(' ')) != std::string::npos) {
                token = c.substring(0, pos);
                Serial.println("Поделено ::" + token +"::");
                if(token[0] == '/'){
                  zapros = token;
                  Serial.println("Запрос найден ::" + zapros +"::");
                }
                c.remove(0, pos + 1);
            }
            endstr = zapros.indexOf("\n");
            while ((pos = zapros.indexOf('_')) != std::string::npos) {
                token = zapros.substring(0, pos);
                Serial.println("Запрос оделено ::" + token +"::");
                if(token[0] == '/'){
                  ob = token;
                  Serial.println("Объект найден ::" + ob +"::");
                  de = zapros.substring(pos+1, endstr);
                  Serial.println("Действие найдено ::" + de +"::");
                }
                zapros.remove(0, pos + 1);
            }
           }
          }
          if(ob[1] == 'l'){
            if(de[1] == 'n'){
              On_L();
            }
            else if(de[1] == 'f'){
              Off_L();
            }
          } 
          else if(ob[1] == 'p'){
            sdel_pr(de.toInt());
          } 

          // если пришла пустая строка значит запрос закончился
          // тогда мы можем отправлять ответ
          if (c2[c2.length()-1] == '\n' && currentLineIsBlank) {
            // отправляем стандартные http заголовки
            client.println("HTTP/1.1 200 OK"); 
            client.println("Content-Type: text/html");
            client.println("Connection: close");   // закрыть соединение после получения ответа
            //client.println("Refresh: 5");   // автоматическое обновление ответа каждые 5 секунд
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");

            // выводим данные каждого аналогово пина 
            client.print("Lamp ");
            if(led_sost){
              client.print(String(dutyCycle/2.55)+"%"); 
            }
            else{
              client.print("OFF"); 
            }
            //client.println("<br />");
            //client.println("<form action=\"on/off\" method=\"post\">");
            //client.println("<button name=\"popup-button\" value=\"upvote\"> ON|OFF </button>");
            //client.println(" </form>");
            //client.println("<br />");
            //client.println("<form action=\"add\" method=\"post\">");
            //client.println("<button name=\"popup-button\" value=\"upvote\"> ADD </button>");
            //client.println(" </form>");
            //client.println("<br />");
            //client.println("<form action=\"sub\" method=\"post\">");
            //client.println("<button name=\"popup-button\" value=\"upvote\"> SUB </button>");
            //client.println(" </form>");
            //client.println("<br />");

            client.println("</html>");     
          break;
        }

        if (c2[c2.length()-1] == '\n') {
            // вы начали новую линию
          currentLineIsBlank = true;
        } else if (c2[c2.length()-1] != '\r') {
            // вы получили символ на текущей линии
          currentLineIsBlank = false;
        }
      }
    }

      // небольшая задержка на то что бы браузер получил данные
    delay(1);

      // закрываем подключение
    client.stop();
    Serial.println("Клиент отключен");
  }
}
