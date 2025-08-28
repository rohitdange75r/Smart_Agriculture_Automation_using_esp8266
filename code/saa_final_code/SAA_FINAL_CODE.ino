#include "DHT.h"
#include "PCF8574.h"
#include <Adafruit_SSD1306.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <MCP3XXX.h>
#include <WiFiClient.h>
#include <Wire.h>

#define DHTPIN 0      // GPIO pin 0 define for INPUT pin of dht sensor
#define DHTTYPE DHT11 // type of the dht sensor DHT11
// #define LED 16        // GPIO pin 16 define used for output pin fo relay control
#define FLOWSENSOR 2 // GPIO pin 2 defined for flow rate input pin ssd1306

// SSID and Password of your WiFi router
const char *ssid = "*";
const char *password = "87654321";

LiquidCrystal_I2C lcd(0x27, 20, 4); // display pin declaration of the i2c
PCF8574 pcf8574(0x38);
DHT dht(DHTPIN, DHTTYPE); // dht pin declaration
MCP3008 adc;              // mcp3208 spi pin declaration

String t_state;
String PumpState;
String temperature_c;

// String PumpState1;
// String PumpState2;
// String PumpState3;

int adc1state;
int adc2state;
int adc3state;
int ButtonState;

long const decimalPrecision = 1; // decimal point declaration
long const ADC_REF = 5.0;        // anolog referance 5v declaration
int SoilMoisture = A0;           // A0 pin declaration for soil moisture sensor
int j = 0;
int prev = 0;
int pres = 0;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000; // time interval of 1 second for flow rate sensor
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;

void IRAM_ATTR pulseCounter()
{
    pulseCount++;
}

ESP8266WebServer server(80); // Server on port 80

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Agriculture WEB Server</title>

    <style>
        html {
            font-family: Arial;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }

        body {
            /* background-color: rgb(96, 184, 24); */
            background-color: white;
            margin-bottom: -10px;
            font-size: 20px;
        }

        h1 {
            /* margin-bottom: 10pc; */
            margin-top: -1pc;
            color: #070812;
            padding: 4vh;
            font-size: 20px;
        }

        .button {
            /* display: inline-block; */
            background-color: #24be16;
            border: none;
            border-radius: 4px;
            color: white;
            padding: 8px 5px;
            text-decoration: none;
            font-size: 20px;
            margin-bottom: 0;
            margin: 2px;

        }

        .button2 {
            background-color: #e00d29;
        }

        .content {
            padding: 50px;
        }

        .card-grid {
            max-width: 500px;
            margin: 0 auto;
            display: grid;
            grid-gap: 2rem;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));

        }

        .card {
            max-height: 300px;
            background-color: rgb(36, 156, 192);
            box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
            margin-bottom: -5.8pc;
            border: none;
            border-radius: 20px;
            vertical-align: middle;
        }

        .card-title {
            font-size: 20px;
            font-weight: bold;
            color: #034078
        }

        .p {
            margin-top: 1pc;
        }


        .units {
            font-size: 20px;
        }

        .phase {
            font-size: 20px;
            vertical-align: middle;
            padding-bottom: 15px;
        }

        .main {
            margin-top: -5pc;
        }
    </style>
</head>

<body>
    <h1>SMART AGRICULTURE AUTOMATION USING ESP8266 NODE MCU</h1>
    <div class="main">
        <div class="content">
            <div class="card-grid">
                <div class="card">
                    <div class="phase">
                        <div id="demo">
                            <div class="p">
                                <div style="color: rgb(12, 8, 17); margin-bottom: -1pc;">Switch State : <span
                                        id="PUMPstate">NA</span>
                                </div><br>
                                <button class="button" type="button" onclick="sendData(1)">PUMP ON</button>
                                <button class="button button2" type="button" onclick="sendData(0)">PUMP OFF</button><br>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="content">
        <div class="card-grid">
            <div class="card">
                <div class="phase">
                    <div class="p">
                        (R)Phase1 Mode : <span id="protection1">NA</span><br>
                        (Y)Phase2 Mode : <span id="protection2">NA</span><br>
                        (B)Phase3 Mode : <span id="protection3">NA</span><br>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="content">
        <div class="card-grid">
            <div class="card">
                <div class="phase">
                    <div class="p">
                        Pump ( Run ) : <span id="protection4">NA</span><br>
                        Pump ( Mode) : <span id="protection5">NA</span><br>
                        Pump (State) : <span id="pumpstate">NA</span><br>
                        Manual Switch : <span id="ManualSwitch">NA</span><br>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="content">
        <div class="card-grid">
            <div class="card">
                <div class="phase">
                    <div class="p">
                        (R)PhaseVoltage1 = <span id="ADCValue1">0</span>
                        <span>V</span><br>
                        (Y)PhaseVoltage2 = <span id="ADCValue2">0</span>
                        <span>V</span><br>
                        (B)PhaseVoltage3 = <span id="ADCValue3">0</span>
                        <span>V</span><br>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="content">
        <div class="card-grid">
            <div class="card">
                <div class="phase">
                    <div class="p">
                        FlowRate = <span id="FlowRate">0</span>
                        <span>L/min</span><br>
                        FlowMilliLitres = <span id="FlowMilliLitres">0</span>
                        <span>mL</span><br>
                        FlowLitres = <span id="FlowLitres">0</span>
                        <span>L</span><br>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="content">
        <div class="card-grid">
            <div class="card">
                <div class="phase">
                    <div class="p">
                        SoilMoisture = <span id="Analog_input">0</span>
                        <span>%</span><br>
                        Humidity = <span id="Humidity">0</span>
                        <span>%</span><br>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div class="content">
        <div class="card-grid">
            <div class="card">
                <div class="phase">
                    <div class="p">
                        (C)Temperature = <span id="Temperature_c">0</span>
                        <sup class="units">°C</sup><br>
                        (F)Temperature = <span id="Temperature_f">0</span>
                        <sup class="units">°F</sup><br>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>

        function sendData(pump) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("PUMPstate").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "setPUMP?PUMPstate=" + pump, true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData1();
        }, 500); //2000mSeconds update rate

        function getData1() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("protection1").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readprotection1", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData2();
        }, 500); //2000mSeconds update rate

        function getData2() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("protection2").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readprotection2", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData3();
        }, 500); //2000mSeconds update rate

        function getData3() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("protection3").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readprotection3", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData4();
        }, 500); //2000mSeconds update rate

        function getData4() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("protection4").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readprotection4", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData5();
        }, 500); //2000mSeconds update rate

        function getData5() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("protection5").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readprotection5", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData6();
        }, 500); //2000mSeconds update rate

        function getData6() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("pumpstate").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readpumpstate", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData7();
        }, 500); //2000mSeconds update rate

        function getData7() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("ADCValue1").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readADC1", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData8();
        }, 500); //2000mSeconds update rate

        function getData8() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("ADCValue2").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readADC2", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData9();
        }, 500); //2000mSeconds update rate

        function getData9() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("ADCValue3").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readADC3", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData10();
        }, 500); //2000mSeconds update rate

        function getData10() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("FlowRate").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readFlowRate", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData11();
        }, 500); //2000mSeconds update rate

        function getData11() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("FlowMilliLitres").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readFlowMilliLitres", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData12();
        }, 500); //2000mSeconds update rate

        function getData12() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("FlowLitres").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readFlowLitres", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData13();
        }, 500); //2000mSeconds update rate

        function getData13() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("Analog_input").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readAnalog", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData14();
        }, 500); //2000mSeconds update rate

        function getData14() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("Humidity").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readHumidity", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData15();
        }, 500); //2000mSeconds update rate

        function getData15() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("Temperature_c").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readTemperature_c", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData16();
        }, 500); //2000mSeconds update rate

        function getData16() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("Temperature_f").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readTemperature_f", true);
            xhttp.send();
        }

        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData17();
        }, 500); //2000mSeconds update rate

        function getData17() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("ManualSwitch").innerHTML =
                        this.responseText;
                }
            };
            xhttp.open("GET", "readmanualswitch", true);
            xhttp.send();
        }

    </script>
</body>

</html>
)=====";

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot()
{
    String s = MAIN_page;             // Read HTML contents
    server.send(200, "text/html", s); // Send web page
}

void handleProtection1()
{
    adc1state = adc.analogRead(0);

    if (adc1state < 485 || adc1state >= 585)
    {
        pcf8574.write(0, LOW);
        PumpState = "LESS/HIGH?";                  // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
    else
    {
        PumpState = "NORMAL";                      // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page

        // int ButtonState = adc.analogRead(3); // Read analog value from A3 pin
        // // For 1st button:
        // if (ButtonState >= 1000 && ButtonState <= 1023)
        // {
        //     pcf8574.write(0, LOW);
        //     Serial.println(" Voltage is Normal Pump OFF ");
        // }
        // // For 2nd button:
        // else if (ButtonState >= 730 && ButtonState <= 760)
        // {

        //     pcf8574.write(0, HIGH);
        //     Serial.println(" Voltage is Normal Pump ON ");
        // }
    }
}

void handleProtection2()
{
    adc2state = adc.analogRead(1);

    if (adc2state < 485 || adc2state >= 585)
    {
        pcf8574.write(0, LOW);
        PumpState = "LESS/HIGH?";                  // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web pag3
    }
    else
    {
        PumpState = "NORMAL";                      // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page

        // int ButtonState = adc.analogRead(3); // Read analog value from A3 pin
        // // For 1st button:
        // if (ButtonState >= 1000 && ButtonState <= 1023)
        // {
        //     pcf8574.write(0, LOW);
        //     Serial.println(" Voltage is Normal Pump OFF ");
        // }
        // // For 2nd button:
        // else if (ButtonState >= 730 && ButtonState <= 760)
        // {

        //     pcf8574.write(0, HIGH);
        //     Serial.println(" Voltage is Normal Pump ON ");
        // }
    }
}

void handleProtection3()
{
    adc3state = adc.analogRead(2);

    if (adc3state < 485 || adc3state >= 585)
    {
        pcf8574.write(0, LOW);
        PumpState = "LESS/HIGH?";                  // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
    else
    {
        PumpState = "NORMAL";                      // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page

        // int ButtonState = adc.analogRead(3); // Read analog value from A3 pin
        // // For 1st button:
        // if (ButtonState >= 1000 && ButtonState <= 1023)
        // {
        //     pcf8574.write(0, LOW);
        //     Serial.println(" Voltage is Normal Pump OFF ");
        // }
        // // For 2nd button:
        // else if (ButtonState >= 730 && ButtonState <= 760)
        // {

        //     pcf8574.write(0, HIGH);
        //     Serial.println(" Voltage is Normal Pump ON ");
        // }
    }
}

void handleProtection4()
{

    if (flowRate < 6)
    {
        PumpState = "DRY RUN";                     // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
    else
    {
        PumpState = "NORMAL RUN";                  // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
}

void handleProtection5()
{
    adc1state = adc.analogRead(0);
    adc2state = adc.analogRead(1);
    adc3state = adc.analogRead(2);

    // if (adc1state < 485 || adc1state >= 585 || adc2state < 485 || adc2state >= 585 || adc3state < 485 || adc3state >= 585 || flowRate < 6)
    if ((adc1state < 485 || adc1state >= 585) || (adc2state < 485 || adc2state >= 585) || (adc3state < 485 || adc3state >= 585))
    {

        PumpState = "PROTECTION";                  // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
    else
    {
        PumpState = "NORMAL";                      // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
}

void handlePumpState()
{
    adc1state = adc.analogRead(0);
    adc2state = adc.analogRead(1);
    adc3state = adc.analogRead(2);

    // if (adc1state < 485 || adc1state >= 585 || adc2state < 485 || adc2state >= 585 || adc3state < 485 || adc3state >= 585 || flowRate < 6)
    if ((adc1state < 485 || adc1state >= 585) || (adc2state < 485 || adc2state >= 585) || (adc3state < 485 || adc3state >= 585))
    {

        PumpState = "OFF";                         // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
    else
    {
        PumpState = "READY FOR ON";                // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
}

void handleManualSwitch()
{
    int ButtonState = adc.analogRead(3); // Read analog value from A0 pin
    // For 1st button:
    if (ButtonState >= 1000 && ButtonState <= 1023)
    {
        pcf8574.write(0, LOW);
        PumpState = "OFF"; // Feedback parameter
        Serial.println(" Voltage is Normal Pump OFF ");
        server.send(200, "text/plane", PumpState); // Send web page
    }
    // For 2nd button:
    else if (ButtonState >= 730 && ButtonState <= 760)
    {
        // float flowrate;
        // int time = 10000;
        // time = millis();
        // if ((flowrate < 6) - millis())
        // {
        //     pcf8574.write(0, LOW);
        // }
        pcf8574.write(0, HIGH);
        Serial.println(" Voltage is Normal Pump ON ");
        PumpState = "ON";                          // Feedback parameter
        server.send(200, "text/plane", PumpState); // Send web page
    }
}

void handleADC1()
{
    adc1state = adc.analogRead(0);
    String adcValue1 = String(((adc1state * ADC_REF) / 4095.0) * 343.93);
    lcd.setCursor(0, 0);
    lcd.print("P1=");
    lcd.print(adcValue1);
    lcd.print("V");

    server.send(200, "text/plane", adcValue1); // Send ADC1 value only to client request
}

void handleADC2()
{
    adc2state = adc.analogRead(1);
    String adcValue2 = String(((adc2state * ADC_REF) / 4095.0) * 343.93);
    lcd.setCursor(0, 1);
    lcd.print("P2=");
    lcd.print(adcValue2);
    lcd.print("V");

    server.send(200, "text/plane", adcValue2); // Send ADC2 value only to client request
}

void handleADC3()
{
    adc3state = adc.analogRead(2);
    String adcValue3 = String(((adc3state * ADC_REF) / 4095.0) * 343.93);
    lcd.setCursor(0, 2);
    lcd.print("P3=");
    lcd.print(adcValue3);
    lcd.print("V");

    server.send(200, "text/plane", adcValue3); // Send ADC3 value only to client request
}

void handleFlowRate()
{

    String FlowRate = String(flowRate);
    lcd.setCursor(0, 3);
    lcd.print("FWR=");
    lcd.print(flowRate);
    lcd.print("L/min");

    server.send(200, "text/plane", FlowRate); // Send flowrate value only to client request
}

void handleFlowMilliLitres()
{

    String FlowMilliLitres = String(flowMilliLitres);

    server.send(200, "text/plane", FlowMilliLitres); // Send flowMIlliLitres value only to client request
}

void handleFlowLitres()
{

    String FlowLitres = String(flowLitres);

    server.send(200, "text/plane", FlowLitres); // Send flowLitres value only to client request
}

void handleSoilMoisture()
{
    int SoilMoisture = analogRead(A0);
    String analog_input = String(SoilMoisture = map(SoilMoisture, 0, 982, 148, 0));

    pres = SoilMoisture;

    if (SoilMoisture > 100)
    {
        SoilMoisture = 100;
    }

    else if (SoilMoisture < 0)
    {
        SoilMoisture = 0;
    }
    prev = SoilMoisture;

    lcd.setCursor(11, 0);
    lcd.print("SM=");
    lcd.print(analog_input);
    lcd.print("%");

    server.send(200, "text/plane", analog_input); // Send soil moisture value only to client request
}

void handlehumidity()
{
    float h = dht.readHumidity();
    String humidity = String(h);

    lcd.setCursor(11, 1);
    lcd.print("H=");
    lcd.print(humidity);
    lcd.print("%");

    server.send(200, "text/plane", humidity); // Send humidity value only to client request
}

void handleTemperatureCelsius()
{
    float t = dht.readTemperature();
    temperature_c = String(t);
    lcd.setCursor(11, 2);
    lcd.print("T=");
    lcd.print(temperature_c);
    lcd.print("*C");

    server.send(200, "text/plane", temperature_c); // Send temperature in celsius value only to client request
}

void handleTemperatureFahrenheit()
{
    float t = dht.readTemperature(true);
    String temperature_f = String(t);

    server.send(200, "text/plane", temperature_f); // Send temperature in fahrenhiet value only to client request
}

void handlePUMP()
{
    adc1state = adc.analogRead(0);
    adc2state = adc.analogRead(1);
    adc3state = adc.analogRead(2);
    ButtonState = adc.analogRead(3); // Read analog value from A3 pin

    PumpState = "OFF";
    t_state = server.arg("PUMPstate"); // Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);

    if (t_state == "0")
    {
        // if ((flowrate < 6)- millis())
        // {
        //     pcf8574.write(0, LOW);
        // }
        pcf8574.write(0, LOW);
        PumpState = "OFF"; // Feedback parameter
    }

    else if (adc1state <= 485 || adc1state >= 585)
    {
        pcf8574.write(0, LOW);
    }

    else if (adc2state <= 485 || adc2state >= 585)
    {
        pcf8574.write(0, LOW);
    }

    else if (adc3state <= 485 || adc3state >= 585)
    {
        pcf8574.write(0, LOW);
    }

    else
    {
        // if (ButtonState >= 730 && ButtonState <= 760)
        // {
        //     pcf8574.write(0, HIGH);
        // }
        // float flowrate;
        // int time = 10000;
        // time = millis();
        // if ((flowrate < 6) - millis())
        // {
        //     pcf8574.write(0, LOW);
        // }
        pcf8574.write(0, HIGH);
        PumpState = "ON"; // Feedback parameter
    }
    server.send(200, "text/plane", PumpState); // Send web page

    // int ButtonState = adc.analogRead(3); // Read analog value from A3 pin
    // // For 1st button:
    // if (ButtonState >= 1000 && ButtonState <= 1023)
    // {
    //     pcf8574.write(0, LOW);
    //     Serial.println(" Voltage is Normal Pump OFF ");
    // }
    // // For 2nd button:
    // else if (ButtonState >= 730 && ButtonState <= 760)
    // {

    //     pcf8574.write(0, HIGH);
    //     Serial.println(" Voltage is Normal Pump ON ");
    // }
}

// void alwaysON()
// {
// if (WiFI.status() == WL_CONNECTED || WiFi.status() != WL_CONNECTED)
// {

//     lcd.setCursor(11, 2);
//     lcd.print("T=");
//     lcd.print(temperature_c);
//     lcd.print("*C");
// }
// }

//==============================================================
//                  SETUP
//==============================================================

void setup(void)
{

    lcd.init();
    lcd.clear();
    lcd.backlight();

    Serial.begin(115200);
    adc.begin();
    dht.begin();
    pcf8574.begin();

    // pinMode(LED, OUTPUT);              // Onboard LED port Direction output
    pinMode(FLOWSENSOR, INPUT_PULLUP); //  flow rate input port declaration

    pulseCount = 0;
    flowRate = 0.0;
    flowMilliLitres = 0;
    totalMilliLitres = 0;
    previousMillis = 0;

    attachInterrupt(digitalPinToInterrupt(FLOWSENSOR), pulseCounter, FALLING);

    WiFi.begin(ssid, password); // Connect to your WiFi router
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) // Wait for connection
    {
        delay(100);
        Serial.print("*");
    }

    // If connection successful show IP address in serial monitor
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); // IP address assigned to your ESP

    server.on("/", handleRoot); // Which routine to handle at root location. This is display page
    server.on("/setPUMP", handlePUMP);

    server.on("/readprotection1", handleProtection1);
    server.on("/readprotection2", handleProtection2);
    server.on("/readprotection3", handleProtection3);
    server.on("/readprotection4", handleProtection4);
    server.on("/readprotection5", handleProtection5);
    server.on("/readpumpstate", handlePumpState);
    server.on("/readmanualswitch", handleManualSwitch);
    server.on("/readADC1", handleADC1);
    server.on("/readADC2", handleADC2);
    server.on("/readADC3", handleADC3);
    server.on("/readFlowRate", handleFlowRate);
    server.on("/readFlowMilliLitres", handleFlowMilliLitres);
    server.on("/readFlowLitres", handleFlowLitres);
    server.on("/readAnalog", handleSoilMoisture);
    server.on("/readHumidity", handlehumidity);
    server.on("/readTemperature_c", handleTemperatureCelsius);
    server.on("/readTemperature_f", handleTemperatureFahrenheit);

    server.begin(); // Start server
    Serial.println("HTTP server started");

    // if (WiFI.status() == WL_CONNECTED || WiFi.status() != WL_CONNECTED)
    // {

    //     lcd.setCursor(11, 2);
    //     lcd.print("T=");
    //     lcd.print(temperature_c);
    //     lcd.print("*C");
    // }

    // if (WiFi.status() == WL_CONNECTED)
    // {
    //     Serial.println("Connected to Wi-Fi");
    //     // Run code for when Wi-Fi is connected here
    //     // lcd.setCursor(11, 2);
    //     // lcd.print("T=");
    //     // lcd.print(temperature_c);
    //     // lcd.print("*C");
    // }
    // else
    // {
    //     Serial.println("Not connected to Wi-Fi");
    //     // Run code for when Wi-Fi is not connected here

    //     lcd.setCursor(11, 2);
    //     lcd.print("T=");
    //     lcd.print(temperature_c);
    //     lcd.print("*C");
    // }
}

//==============================================================
//                     LOOP
//==============================================================
void loop(void)
{
    server.handleClient(); // Handle client requests

    currentMillis = millis();
    if (currentMillis - previousMillis > interval)
    {

        // delay(30000);
        pulse1Sec = pulseCount;
        pulseCount = 0;

        // Because this loop may not complete in exactly 1 second intervals we calculate
        // the number of milliseconds that have passed since the last execution and use
        // that to scale the output. We also apply the calibrationFactor to scale the output
        // based on the number of pulses per second per units of measure (litres/minute in
        // this case) coming from the sensor.
        flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
        previousMillis = millis();

        // Divide the flow rate in litres/minute by 60 to determine how many litres have
        // passed through the sensor in this 1 second interval, then multiply by 1000 to
        // convert to millilitres.
        flowMilliLitres = (flowRate / 60) * 1000;
        flowLitres = (flowRate / 60);

        // Add the millilitres passed in this second to the cumulative total
        totalMilliLitres += flowMilliLitres;
        totalLitres += flowLitres;

        // int phase = adc.analogRead(0);
        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("adc=");
        // lcd.print(phase);
        // delay(500);
    }
}