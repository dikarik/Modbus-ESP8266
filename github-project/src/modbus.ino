#include <ESP8266WiFi.h>
#include <ModbusIP_ESP8266.h> // Если не компилируется, замените на <ModbusIP.h>

// --- НАСТРОЙКИ ПОДКЛЮЧЕНИЯ ---
const char* ssid     = "vi fi";     
const char* password = "58585858";  

// --- АДРЕСА РЕГИСТРОВ ДЛЯ SIMP Light ---
const int SENSOR_REG = 100; // Чтение: Аналоговый вход A0
const int WIFI_REG   = 101; // Чтение: Сигнал Wi-Fi (RSSI)
const int LED_REG    = 104; // Чтение и Запись: Управление светодиодом (1 - Вкл, 0 - Выкл)

ModbusIP mb; // Объект Modbus TCP

void setup() {
  Serial.begin(115200);

  // Подключение к Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("");
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Ожидаем 192.168.0.105

  // Инициализация Modbus TCP Сервера
  mb.server(); 

  // Добавление Holding Registers (все типы 03/06)
  mb.addHreg(SENSOR_REG, 0); 
  mb.addHreg(WIFI_REG, 0);   
  mb.addHreg(LED_REG, 0);    // Регистр 104 для управления

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Выключен при старте
}

void loop() {
  // Обработка запросов от SCADA
  mb.task();

  if (WiFi.status() == WL_CONNECTED) {
    // 1. Запись уровня сигнала с А0 в регистр 100
    mb.Hreg(SENSOR_REG, analogRead(A0));

    // 2. Запись уровня Wi-Fi в регистр 101
    mb.Hreg(WIFI_REG, abs(WiFi.RSSI()));

    // 3. Управление светодиодом через регистр 104
    // Если в SCADA записали в 104-й регистр число > 0, включаем свет
    if (mb.Hreg(LED_REG) > 0) {
      digitalWrite(LED_BUILTIN, LOW);  // Включить (специфика ESP8266)
    } else {
      digitalWrite(LED_BUILTIN, HIGH); // Выключить
    }
  }

  delay(10); // Пауза для стабильности
}