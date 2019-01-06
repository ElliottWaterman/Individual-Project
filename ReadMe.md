# Individual Project (University of Reading)

## Smart Boa Snake Basking Station (SBSBS)

### Sensors and components
* RFID: RFIDRW-E-TTL (Priority1design)
* Temperature and Humidity: DHT22
* Weight (Amplifier and Load Cell): HX711 and 3kg Load Cell
* Real Time Clock: DS3231
* GSM: SIM900 GPRS Shield

### Libraries
* RFID: SoftwareSerial
* Temperature and Humidity: SimpleDHT
* Weight (Amplifier and Load Cell): Queuetue\_HX711\_Library (modified) & HX711\_ADC (not used)
* Real Time Clock: DS3231
* GSM: GSM

## Twilio Server: SMS Receiver and Report Viewer

* Java application using Twilio API to create SMS receiving server and data report viewer
* ngrok program used to tunnel localhost port to a secure URL
* ngrok connects Java server on port 4567 to a secure URL
