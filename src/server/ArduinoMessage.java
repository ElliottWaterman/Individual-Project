package server;

import java.util.ArrayList;

/**
 * Class to define the structure of a text message sent from the
 * Arduino / SIM900 module that contains sensor data.
 * @author Elliott Waterman
 */
public class ArduinoMessage {
	/**
	 * The number of identity parameters in an Arduino message.
	 */
	private static final int NUMBER_OF_PARAMETERS = 2;
	/**
	 * The number of sensor readings sent in a text message.
	 */
	private static final int NUMBER_OF_SENSOR_READINGS = 4;
	/**
	 * Maximum temperature before reading or value is incorrect (Mauritius highest 40C).
	 */
	private static final int MAX_TEMPERATURE = 60;
	/**
	 * Minimum temperature before reading or value is incorrect (Mauritius lowest 5C).
	 */
	private static final int MIN_TEMPERATURE = -10;
	/**
	 * Maximum humidity before reading or value is incorrect (Sensor highest 100%).
	 */
	private static final int MAX_HUMIDITY = 100;
	/**
	 * Minimum humidity before reading or value is incorrect (Sensor lowest 0%).
	 */
	private static final int MIN_HUMIDITY = 0;
	/**
	 * Separator for a comma separator value.
	 */
	private static final String CSV_SEPARATOR = ",";
	
	private String messageSid;
	private String phoneNumber;
	private Long epochMillis;
	private Float temperature;
	private Float humidity;
	private Float weight;
	private String snakeRFID;
	private ArrayList<String> skinkRFIDs;
	
	/**
	 * Constructor to parse a CSV line from either an incoming text message or the storage file.
	 * @param CSVString A CSV data line from a text message or storage file.
	 */
	public ArduinoMessage(String CSVString) {
		this.messageSid = null;
		this.phoneNumber = null;
		this.epochMillis = null;
		this.temperature = null;
		this.humidity = null;
		this.weight = null;
		this.snakeRFID = null;
		this.skinkRFIDs = new ArrayList<String>();
		
		// Parse CSV text into variable values
		this.parseCSVText(CSVString);
	}
	
	/**
	 * Function to parse an incoming text message body or storage file line that is in CSV format.
	 * @param CSVText An incoming text message or storage file line to parse.
	 */
	public void parseCSVText(String CSVText) {
		// Check string is not null
		if (CSVText != null) {
			// Check string is a CSV line
			if (CSVText.contains(CSV_SEPARATOR)) {
				// Split CSV data by comma
				String CSVParameters[] = CSVText.split(CSV_SEPARATOR);
				// Check number of parameters and sensor readings match or is greater than the defined amount
				if (CSVParameters.length >= (NUMBER_OF_PARAMETERS + NUMBER_OF_SENSOR_READINGS)) {
					// Parse each string to primitive types
					// Twilio information
					this.messageSid = CSVParameters[0];
					this.phoneNumber = CSVParameters[1];
					
					// SBSBS information
					this.epochMillis = convertStringToLong(CSVParameters[2]);
					this.temperature = convertStringToFloat(CSVParameters[3]);
					this.humidity = convertStringToFloat(CSVParameters[4]);
					this.weight = convertStringToFloat(CSVParameters[5]);
					this.snakeRFID = CSVParameters[6];
					// Add each Skink tag to an array list
					if (CSVParameters.length >= (1 + NUMBER_OF_PARAMETERS + NUMBER_OF_SENSOR_READINGS)) {
						for (int i = 7; i < CSVParameters.length; i++) {
							this.skinkRFIDs.add(CSVParameters[i]);
						}						
					}
				}
			}
		}
	}
	
	/**
	 * Function to check all data variables are not null.
	 * @return True if all variables are available, false if not.
	 */
	public boolean checkDataExists() {
		if (this.messageSid == null) {
			return false;
		}
		if (this.phoneNumber == null) {
			return false;
		}
		if (this.epochMillis == null) {
			return false;
		} 
		else if (this.epochMillis.longValue() <= 0) {
			return false;
		}
		if (this.temperature == null) {
			return false;
		}
		else if ((this.temperature.floatValue() < MIN_TEMPERATURE) || 
				 (this.temperature.floatValue() > MAX_TEMPERATURE)) {
			return false;
		}
		if (this.humidity == null) {
			return false;
		}
		else if ((this.humidity.floatValue() < MIN_HUMIDITY) || 
				 (this.humidity.floatValue() > MAX_HUMIDITY)) {
			return false;
		}
		if (this.weight == null) {
			return false;
		}
		else if (this.weight.floatValue() < 0) {
			return false;
		}
		if (this.snakeRFID == null) {
			return false;
		}
		if (this.skinkRFIDs == null) {
			return false;
		}
		
		return true;
	}
	
	/**
	 * Function to parse all variables to a CSV string.
	 * @return A CSV string containing all data from variables.
	 */
	public String parseToCSVString() {
		StringBuilder CSVString = new StringBuilder();
		
		CSVString.append(this.messageSid);
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.phoneNumber);
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.epochMillis.longValue());
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.temperature.floatValue());
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.humidity.floatValue());
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.weight.floatValue());
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.snakeRFID);
		CSVString.append(CSV_SEPARATOR);
		for (int index = 0; index < this.skinkRFIDs.size(); index++) {
			CSVString.append(this.skinkRFIDs.get(index));
			// Print comma until last tag printed
			if (index < (this.skinkRFIDs.size() - 1)) {
				CSVString.append(CSV_SEPARATOR);
			}
		}
		
		return CSVString.toString(); 
	}
	
	/**
	 * Getter function to return the message Sid.
	 * @return The Sid of the message.
	 */
	public String getMessageSid() {
		return messageSid;
	}
	
	/**
	 * Setter function to assign the message Sid.
	 * @param messageSid The Sid of the message.
	 */
	public void setMessageSid(String messageSid) {
		this.messageSid = messageSid;
	}
	
	/**
	 * Getter function to return the received from phone number.
	 * @return The received from phone number.
	 */
	public String getPhoneNumber() {
		return phoneNumber;
	}
	
	/**
	 * Setter function to assign the received from phone number.
	 * @param phoneNumber The received from phone number.
	 */
	public void setPhoneNumber(String phoneNumber) {
		this.phoneNumber = phoneNumber;
	}
	
	/**
	 * Getter function to return the epoch milliseconds when the 
	 * snake RFID tag was read.
	 * @return The milliseconds since the epoch.
	 */
	public Long getEpochMillis() {
		return epochMillis;
	}
	
	/**
	 * Setter function to assign the epoch milliseconds when the 
	 * snake RFID tag was read.
	 * @param epochMillis The milliseconds since the epoch.
	 */
	public void setEpochMillis(Long epochMillis) {
		this.epochMillis = epochMillis;
	}
	
	/**
	 * Getter function to return the temperature.
	 * @return The temperature taken when the snake RFID was read.
	 */
	public Float getTemperature() {
		return temperature;
	}
	
	/**
	 * Setter function to assign the temperature.
	 * @param temperature The temperature in degrees Celsius.
	 */
	public void setTemperature(Float temperature) {
		this.temperature = temperature;
	}
	
	/**
	 * Getter function to return the humidity.
	 * @return The humidity taken when the snake RFID was read.
	 */
	public Float getHumidity() {
		return humidity;
	}
	
	/**
	 * Setter function to assign the humidity.
	 * @param humidity The relative humidity as a percentage.
	 */
	public void setHumidity(Float humidity) {
		this.humidity = humidity;
	}
	
	/**
	 * Getter function to return the weight of the snake.
	 * @return The measured weight of the snake in grams.
	 */
	public Float getWeight() {
		return weight;
	}
	
	/**
	 * Setter function to assign the weight of the snake.
	 * @param weight The weight of the snake in grams.
	 */
	public void setWeight(Float weight) {
		this.weight = weight;
	}
	
	/**
	 * Getter function to return the snake RFID.
	 * @return String of the snake RFID tag.
	 */
	public String getSnakeRFID() {
		return snakeRFID;
	}
	
	/**
	 * Setter function to assign the snake RFID tag.
	 * @param snakeRFID The snake RFID tag to set.
	 */
	public void setSnakeRFID(String snakeRFID) {
		this.snakeRFID = snakeRFID;
	}
	
	/**
	 * Getter function to return an array list of Skink RFID tags.
	 * @return Array list of Skink RFID tags.
	 */
	public ArrayList<String> getSkinkRFIDs() {
		return skinkRFIDs;
	}
	
	/**
	 * Setter function to assign the RFID tags of Skinks.
	 * @param skinkRFIDs The array list of Skink RFID tags to set.
	 */
	public void setRFID(ArrayList<String> skinkRFIDs) {
		this.skinkRFIDs = skinkRFIDs;
	}
	
	/**
	 * Function to convert a String into a Long.
	 * @param reading A text message comma separated value.
	 * @return A Long if parse function worked, null if not.
	 */
	private static Long convertStringToLong(String reading) {
		Long convertedReading = null;
		try {
			convertedReading = Long.parseLong(reading);
		} catch (NumberFormatException nfe) {
			nfe.printStackTrace();
		}
		return convertedReading;
	}
	
	/**
	 * Function to convert a String into a Float.
	 * @param reading A text message comma separated value.
	 * @return A Float if parse function worked, null if not.
	 */
	private static Float convertStringToFloat(String reading) {
		Float convertedReading = null;
		try {
			convertedReading = Float.parseFloat(reading);
		} catch (NumberFormatException nfe) {
			nfe.printStackTrace();
		}
		return convertedReading;
	}
	
}	// End class ArduinoMessage
