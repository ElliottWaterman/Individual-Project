package TextMessageToStorage;

import com.twilio.Twilio;
import com.twilio.base.ResourceSet;
import com.twilio.rest.api.v2010.account.Message;

public class Example {
	// Find your Account Sid and Auth Token at twilio.com/console
	public static final String ACCOUNT_SID = "AC397aad87594399d53155d77917a49b37";
	public static final String AUTH_TOKEN = System.getenv("TWILIO_AUTH_TOKEN");

	public static void main(String[] args) {
		Twilio.init(ACCOUNT_SID, AUTH_TOKEN);

		// Create and send a message
//		Message message = Message
//				.creator(new PhoneNumber("+447500000000"), // to my number
//						new PhoneNumber("+441233800093"), // from Twilio number
//						"Where's Wallace?")
//				.create();
//
//		System.out.println(message.getSid());

		// Use to display all parameter keywords
//		System.out.println("");
//		System.out.println(req.queryParams().size());
//		for (String text : req.queryParams()) {
//			System.out.println(text);
//		}

    	// Array because of multiple pages of same message (over 160 characters)
//    	System.out.println(req.queryParamsValues("Body").length);
//    	for (String text : req.queryParamsValues("Body")) {
//    		System.out.println(text);
//    	}

		// Get specific message
//		Message message = Message.fetcher("SM77f82fd086a1bac322af361c0c4d0187").fetch();
//		System.out.println(message.getFrom());
//		System.out.println(message.getBody());

		// Get all messages
		ResourceSet<Message> messages = Message.reader().read();
		for (Message record : messages) {
			System.out.println(record.getSid());
			System.out.println(record.getDateSent());
			System.out.println(record.getBody());
		}
	}
}