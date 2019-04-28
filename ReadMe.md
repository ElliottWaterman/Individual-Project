# Individual Project (University of Reading)

## Smart Boa Snake Basking Station (SBSBS)

### Sensors and components
* RFID: RFIDRW-E-TTL (Priority1design)
* Temperature and Humidity: DHT22
* Weight (Amplifier and Load Cell): HX711 and 3kg Load Cell
* RTC: DS3231
* GSM: SIM900 GPRS Shield
* Micro SD Card: B026 POD

### Libraries
* RFID: Created with SoftwareSerial
* Temperature and Humidity: SimpleDHT
* Weight (Amplifier and Load Cell): Queuetue\_HX711\_Library (modified)
* RTC: DS3231
* GSM: Created with SoftwareSerial
* Micro SD Card: FAT16

## Twilio Server: SMS Receiver and Report Viewer

* Java application using Spark web framework to deal with HTTP requests
* Twilio API is included to receive SMS messages
* ngrok program securely tunnels between localhost port 4567 and a secure URL
* Twilio Console has an SMS webhook address in form of a HTTP POST request set to the ngrok secure URL
  * https://www.twilio.com/login

### Google Drive Credentials

* Download credentials file and save in the assets folder
  * https://console.developers.google.com/apis/credentials?project=project-id-0590277823671900620
  * https://console.developers.google.com/apis/library?project=project-id-0590277823671900620

