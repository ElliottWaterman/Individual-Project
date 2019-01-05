package server;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import com.twilio.twiml.MessagingResponse;
import static spark.Spark.*;
//import static spark.Spark.get;
//import static spark.Spark.post;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Type;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URLEncodedUtils;

/**
 * Class to receive and process Twilio Webhook GET and POST requests. Incoming text messages
 * to a Twilio phone number will run POST function on this local server. Use ngrok to give 
 * public address to this local server.
 * 
 * @author Elliott Waterman
 */
public class SMSReceiverReportViewer {
	
	/**
	 * A TwiML XML string that defines when no message is sent as a response.
	 */
	private static final String NO_MESSAGE_REPLY = new MessagingResponse.Builder().build().toXml();
	
	/**
	 * Data storage file for the Smart Boa snake basking station.
	 */
	private static final File STORAGE_FILE = new File("SBSBS.csv");
	/**
	 * Separator for a comma separator value.
	 */
	private static final String CSV_SEPARATOR = ",";
	
	/**
	 * Java main function to run.
	 * @param args Any arguments passed to the program.
	 */
    public static void main(String[] args) {
    	// GET requests
        get("/", (req, res) -> {
        	// Read in data from CSVstorage file
        	ArrayList<ArduinoMessage> listOfMessages = readCSVFile();
        	
        	// Create HTML page here
        	StringBuilder htmlBase = new StringBuilder("<!doctype html><html lang=\"en\">" +
        		"<head>" +
        		  "<meta charset=\"utf-8\">" +
				  "<title>Smart Boa Snake Basking Station Data Report</title>" +
				  "<meta name=\"description\" content=\"Data report utilising SMS received from the smart basking station\">" +
				  "<meta name=\"author\" content=\"Elliott Waterman\">" +
				  //"<link rel=\"stylesheet\" href=\"styles.css\">" +
				  "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/tabulator/4.1.4/css/tabulator.min.css\">" +
				  "<script type=\"text/javascript\" src=\"https://cdnjs.cloudflare.com/ajax/libs/tabulator/4.1.4/js/tabulator.min.js\"></script>" +
				  "<script type=\"text/javascript\" src=\"assets/main.js\"" +
				"</head>" +
				"<body>");
        	
    		// Read data in and parse to HTML
			for (ArduinoMessage message : listOfMessages) {
				//htmlBase.append();
				message.getMessageSid();
	            message.getPhoneNumber();
	            message.getEpochMillis();
	            message.getRFID();
	            message.getTemperature();
	            message.getWeight();
			}
			
			// Add body end and HTML end
			htmlBase.append("<div id=\"report-table\"></div></body></html>");
        	
			// Return completed HTML string
        	return htmlBase.toString();
        });
        // TODO: Display all stored text data

        // POST requests
        post("/sms", (req, res) -> {
        	// By calling either req.body() or req.queryParams() the message is taken and cleared.
        	// This means that only one function should be used. The other function returns null!

        	// Get the name value pairs of message parameters (includes text message body)
        	String messageParameters = req.body();
        	System.out.println("Message Parameters:");
        	System.out.println(messageParameters);
        	
        	// Parse the information into a list of name value pairs
        	List<NameValuePair> listOfPairs = URLEncodedUtils.parse(messageParameters, Charset.defaultCharset());
        	System.out.println("Number of parameter pairs: " + listOfPairs.size());
        	
        	// Convert the list into a map of string pairs
            Map<String, String> parameterMap = listToMap(listOfPairs);
            
            // Check message sid and phone number (from)
            String messageSid = parameterMap.get("MessageSid");
            String fromPhoneNumber = parameterMap.get("From");
            String bodyText = parameterMap.get("Body");
            if ((messageSid == null) || (fromPhoneNumber == null) || (bodyText == null)) {
            	// TODO: Store received message anyway
            	return NO_MESSAGE_REPLY;
            }
            System.out.println("Message SID: " + messageSid);
            System.out.println("From Phone Number: " + fromPhoneNumber);
            System.out.println("Message Body: " + bodyText);
            
            // Create complete CSV line of data
            StringBuilder CSVLine = new StringBuilder();
            CSVLine.append(messageSid);
            CSVLine.append(CSV_SEPARATOR);
            CSVLine.append(fromPhoneNumber);
            CSVLine.append(CSV_SEPARATOR);
            CSVLine.append(bodyText);
            
            // Parse map of message parameters to Arduino Message class object
            ArduinoMessage message = new ArduinoMessage(CSVLine.toString());
            System.out.println(message.getMessageSid());
            System.out.println(message.getPhoneNumber());
            System.out.println(message.getEpochMillis());
            System.out.println(message.getRFID());
            System.out.println(message.getTemperature());
            System.out.println(message.getWeight());
            
            // Check storage file exists
            if (!STORAGE_FILE.exists()) {
            	// Create a new file
            	STORAGE_FILE.createNewFile();
            }
            // Check storage file is a file
            if (!STORAGE_FILE.isFile()) {
            	// TODO: clean exit
            	return NO_MESSAGE_REPLY;
            }
            // Check storage file can be read
            if (!STORAGE_FILE.canRead()) {
            	// TODO: clean exit
            	return NO_MESSAGE_REPLY;
            }
            // Check storage file can be written to
            if (!STORAGE_FILE.canWrite()) {
            	// TODO: clean exit
            	return NO_MESSAGE_REPLY;
            }
            
            // Store to JSON
            try {
            	// Append message to storage file
            	appendCSVFile(message);
            	System.out.println("Arduino Message saved to storage file.");
            	System.out.println(STORAGE_FILE.getAbsolutePath());
            	System.out.println("");
            } catch (IOException ioXcp) {
            	// TODO: clean exit
            	ioXcp.printStackTrace();
            	return NO_MESSAGE_REPLY;
            }
            
            return NO_MESSAGE_REPLY;
        });
    }
    
    /**
     * Function to convert a List of name value pairs into a Map of string pairs.
     * @param listOfPairs List of name value pairs.
     * @return A Map of key and value strings.
     */
    private static Map<String, String> listToMap(final List<NameValuePair> listOfPairs) {
    	Map<String, String> mapToReturn = new HashMap<>();
        for (NameValuePair pair : listOfPairs) {
        	mapToReturn.put(pair.getName(), pair.getValue());
        }
        return mapToReturn;
    }
    
    /**
     * Function to append a single message to the storage file.
     * @param message A single message to be stored.
     * @return True if the message was appended, exception if not.
     * @throws IOException An IO exception caused by file writer.
     */
    private static boolean appendCSVFile(ArduinoMessage message) throws IOException {
    	if (message == null) {
    		return false;
    	}
    	if (!message.checkDataExists()) {
    		return false;
    	}
    	
    	String CSVString = message.parseToCSVString();
    	
    	// Create file writer with path to the storage file (true appends to end)
    	FileWriter writer = new FileWriter(STORAGE_FILE, true);
    	writer.append(CSVString);
    	writer.append(System.lineSeparator());
    	writer.close();
    	
    	return true;
    }
    
    /**
     * Function to append a single message to the storage file.
     * @param message A single message to be stored.
     * @return True if the message was appended, exception if not.
     * @throws IOException An IO exception caused by file writer.
     */
    private static ArrayList<ArduinoMessage> readCSVFile() throws IOException {
    	// Create arraylist of Arduino messages
    	ArrayList<ArduinoMessage> listOfMessages = new ArrayList<ArduinoMessage>();
    	
    	// Create buffered reader and file reader with a path to the storage file
    	BufferedReader bufferedReader = new BufferedReader(new FileReader(STORAGE_FILE));
    	
    	// Read first line from CSV file
    	String inputLine = bufferedReader.readLine();
    	
    	// Loop while more lines are to be read
    	while (inputLine != null) {
    		// Create new arduino message and fill with values
    		ArduinoMessage message = new ArduinoMessage(inputLine);
    		
    		// Add message to the list of messages
    		listOfMessages.add(message);
    		
    		// Read next line of data before loop ends
    		inputLine = bufferedReader.readLine();
    	}
    	
    	bufferedReader.close();
    	
    	return listOfMessages;
    }
    
    
    
    // TODO: Remove below
    
    /**
     * Function to write a list of message to a JSON file.
     * @param messageList The list of messages to be stored.
     * @throws IOException An IO exception caused by file writer.
     */
    private static boolean writeJSON(List<ArduinoMessage> messageList) throws IOException {
    	// Create Gson builder and create Gson object
    	GsonBuilder builder = new GsonBuilder();
    	Gson gson = builder.create();
    	
    	// Create file writer with a path to the storage file
    	FileWriter writer = new FileWriter(STORAGE_FILE);
    	
    	// Write message list to the storage file
    	writer.write(gson.toJson(messageList));
    	writer.close();
    	
    	return true;
	}
    
    /**
     * Function to read a list of message from a JSON file.
     * @return The list of messages read from a JSON file.
     * @throws FileNotFoundException A file not found exception caused when storage file does not exist.
     */
    private static List<ArduinoMessage> readJSON() throws FileNotFoundException {
    	// Create Gson builder and create Gson object
    	GsonBuilder builder = new GsonBuilder();
    	Gson gson = builder.create();
    	
    	// Create buffered reader and file reader with a path to the storage file
    	BufferedReader bufferedReader = new BufferedReader(new FileReader(STORAGE_FILE));
    	
    	// Find the type of list the messages are in
    	Type messageListType = new TypeToken<ArrayList<ArduinoMessage>>(){}.getType();
    	
    	// Read a list of messages from the storage file
    	List<ArduinoMessage> messageList = gson.fromJson(bufferedReader, messageListType);
    	return messageList;
    }
    
    /**
     * Function to write a single message to a JSON file containing a list of messages.
     * @param message A single message to be stored.
     * @throws IOException An IO exception caused by file writer.
     */
    private static boolean appendJSON(ArduinoMessage message) throws IOException {
    	// Create Gson builder and create Gson object
    	GsonBuilder builder = new GsonBuilder();
    	Gson gson = builder.create();
    	
    	// Create file writer with path to the storage file (true appends to end)
    	FileWriter writer = new FileWriter(STORAGE_FILE, true);
    	
    	// Find the type of list the messages are in
    	Type messageListType = new TypeToken<ArrayList<ArduinoMessage>>(){}.getType();
    	
    	// Append a message to the storage file
    	ArrayList<ArduinoMessage> messageList = new ArrayList<ArduinoMessage>();
    	messageList.add(message);
    	writer.append(gson.toJson(messageList, messageListType));
    	writer.close();
    	
    	// TODO: Currently just adds a new "list" to JSON file rather than append a single message
    	
    	return true;
	}
    
}	// End class StoreIncomingSMS
