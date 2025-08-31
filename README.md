🌱 Smart Agriculture Automation using ESP8266 (Final Year Project)

📌 Introduction

This project focuses on **protecting and automating a three-phase water pump** using an *ESP8266 NodeMCU*. The system ensures reliable farm irrigation by monitoring multiple parameters, providing pump safety, and offering remote control via a web interface.

🔧 Features

1. Three-Phase Voltage Protection – Automatically stops the pump if any phase voltage is too low or too high.
2. Dry Run Protection – Prevents pump damage by turning it off when water levels are low or the pump runs dry.
3. Water Usage Monitoring – Accurately measures how much water is consumed on the farm (in litres).
4. Environmental Monitoring – Tracks *temperature and humidity*, essential for new plant growth.
5. Web Server Integration – Sends all data to a web server, where users can monitor conditions and *control the pump (ON/OFF)* remotely.

💡 Motivation

This project was inspired by the *real challenges I faced in my farm*:

* Voltage mismatch damaging the pump
* Pump running dry due to low water levels
* Difficulty in measuring water usage
* Needing accurate temperature & humidity readings for new crops

To solve these issues, I decided to design a **smart farm automation system**. With ESP8266 and IoT integration, the solution makes farming **more efficient, safer for equipment, and easier to manage remotely**.

🖥️ How It Works

* The ESP8266 continuously monitors **three-phase voltage, water level, flow rate, temperature, and humidity**.
* Protection mechanisms stop the pump automatically in unsafe conditions.
* A *web interface* allows farmers to view data and manually control the pump.

📊 Benefits

✔️ Prevents pump damage
✔️ Saves water and improves usage efficiency
✔️ Helps in crop management through environmental monitoring
✔️ Remote access and control through IoT
