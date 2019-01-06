package server;

/**
 * Class to define the structure of a text message sent from the
 * Arduino / SIM900 module that contains sensor data.
 * @author Elliott Waterman
 */
public class ArduinoMessage {
	/**
	 * The number of identity parameters in an arduino message.
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
	 * Separator for a comma separator value.
	 */
	private static final String CSV_SEPARATOR = ",";
	
	private String messageSid;
	private String phoneNumber;
	private Long epochMillis;
	private String RFID;
	private Float temperature;
	private Float weight;
	
	/**
	 * Constructor to parse a CSV line from either an incoming text message or the storage file.
	 * @param CSVString A CSV data line from a text message or storage file.
	 */
	public ArduinoMessage(String CSVString) {
		this.messageSid = null;
		this.phoneNumber = null;
		this.epochMillis = null;
		this.RFID = null;
		this.temperature = null;
		this.weight = null;
		
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
			if (CSVText.contains(",")) {
				// Split CSV data by comma
				String CSVParameters[] = CSVText.split(",");
				// Check number of parameters and sensor readings match the defined amount
				if (CSVParameters.length == (NUMBER_OF_PARAMETERS + NUMBER_OF_SENSOR_READINGS)) {
					// Parse each string and assign parameter and sensor readings to variables  
					this.messageSid = CSVParameters[0];
					this.phoneNumber = CSVParameters[1];
					this.epochMillis = convertStringToLong(CSVParameters[2]);
					this.RFID = CSVParameters[3];
					this.temperature = convertStringToFloat(CSVParameters[4]);
					this.weight = convertStringToFloat(CSVParameters[5]);
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
		if (this.RFID == null) {
			return false;
		}
		if (this.temperature == null) {
			return false;
		}
		else if ((this.temperature.floatValue() < MIN_TEMPERATURE) || 
				 (this.temperature.floatValue() > MAX_TEMPERATURE)) {
			return false;
		}
		if (this.weight == null) {
			return false;
		}
		else if (this.weight.floatValue() < 0) {
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
		CSVString.append(this.RFID);
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.temperature.floatValue());
		CSVString.append(CSV_SEPARATOR);
		CSVString.append(this.weight.floatValue());
		
		return CSVString.toString(); 
	}
	
	/**
	 * Getter function to return the message Sid.
	 * @return The message Sid.
	 */
	public String getMessageSid() {
		return messageSid;
	}
	
	/**
	 * Setter function to assign message Sid.
	 * @param messageSid Sid of the message.
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
	 * Getter function to return epoch milliseconds.
	 * @return Milliseconds since the epoch.
	 */
	public Long getEpochMillis() {
		return epochMillis;
	}
	
	/**
	 * Setter function to assign epoch milliseconds.
	 * @param epochMillis Milliseconds since the epoch.
	 */
	public void setEpochMillis(Long epochMillis) {
		this.epochMillis = epochMillis;
	}
	
	/**
	 * Getter function to return RFID.
	 * @return Identification of Radio Frequency.
	 */
	public String getRFID() {
		return RFID;
	}
	
	/**
	 * Setter function to assign the RFID.
	 * @param RFID Identification of Radio Frequency.
	 */
	public void setRFID(String RFID) {
		this.RFID = RFID;
	}
	
	/**
	 * Getter function to return RFID.
	 * @return Identification of Radio Frequency.
	 */
	public Float getTemperature() {
		return temperature;
	}
	
	/**
	 * Setter function to assign the temperature.
	 * @param temperature The temperature in degrees celsius.
	 */
	public void setTemperature(Float temperature) {
		this.temperature = temperature;
	}
	
	/**
	 * Getter function to return the weight.
	 * @return The measured weight of a snake.
	 */
	public Float getWeight() {
		return weight;
	}
	
	/**
	 * Setter function to assign the weight.
	 * @param weight The weight in grams (or kilograms).
	 */
	public void setWeight(Float weight) {
		this.weight = weight;
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
			
		}
		return convertedReading;
	}
	
}	// End class ArduinoMessage
