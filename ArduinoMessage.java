package TextMessageToStorage;

/**
 * Class to define the structure of a text message sent from the
 * Arduino / SIM900 module that contains sensor data.
 * 
 * @author Elliott Waterman
 */
public class ArduinoMessage {
	/**
	 * The number of sensor readings sent in the text message.
	 */
	private static final int NUMBER_OF_SENSOR_READINGS = 4;
	
	private String messageSid;
	private String phoneNumber;
	private Long epochMillis;
	private String RFID;
	private Float temperature;
	private Float weight;
	
	/**
	 * Constructor to assign message Sid and from phone number.
	 * @param messageSid The Sid of the message.
	 * @param phoneNumber The received from phone number.
	 */
	public ArduinoMessage(String messageSid, String phoneNumber, String bodyText) {
		this.messageSid = messageSid;
		this.phoneNumber = phoneNumber;
		this.epochMillis = null;
		this.RFID = null;
		this.temperature = null;
		this.weight = null;
		
		this.parseCSVString(bodyText);
	}
	
	public ArduinoMessage(String messageSid, String phoneNumber, long epochMillis, 
						  String RFID, float temperature, float weight) {
		this.messageSid = messageSid;
		this.phoneNumber = phoneNumber;
		this.epochMillis = epochMillis;
		this.RFID = RFID;
		this.temperature = temperature;
		this.weight = weight;
	}
	
	/**
	 * Function to parse an incoming text message body.
	 * @param textMessageBody The body of text message to parse.
	 */
	public void parseCSVString(String textMessageBody) {
		// Check body is not null
		if (textMessageBody != null) {
			// Check body is a CSV string
			if (textMessageBody.contains(",")) {
				// Split body by comma
				String sensorReadings[] = textMessageBody.split(",");
				// Check number of sensor readings match the defined amount
				if (sensorReadings.length == NUMBER_OF_SENSOR_READINGS) {
					// Parse strings and assign sensor readings to variables  
					this.epochMillis = convertStringToLong(sensorReadings[0]);
					this.RFID = sensorReadings[1];
					this.temperature = convertStringToFloat(sensorReadings[2]);
					this.weight = convertStringToFloat(sensorReadings[3]);
				}
			}
		}
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
