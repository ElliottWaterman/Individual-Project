package TextMessageToStorage;

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
public class StoreIncomingSMS {
	
	/**
	 * A TwiML XML string that defines when no message is sent as a response.
	 */
	private static final String NO_MESSAGE_REPLY = new MessagingResponse.Builder().build().toXml();
	
	/**
	 * Data storage file for the Smart Boa snake basking station.
	 */
	private static final File STORAGE_FILE = new File("SBSBS.json");
	
	/**
	 * Java main function to run.
	 * @param args Any arguments passed to the program.
	 */
    public static void main(String[] args) {
    	// GET requests
        get("/", (req, res) -> "Hello Web");
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
            
            // Parse map of message parameters to Arduino Message class object
            ArduinoMessage message = new ArduinoMessage(messageSid, fromPhoneNumber, bodyText);
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
            	appendJSON(message);
            	System.out.println("Arduino Message saved to JSON file.");
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
