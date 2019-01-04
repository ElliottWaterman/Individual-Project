package TextMessageToStorage;

import com.twilio.twiml.MessagingResponse;
import com.twilio.twiml.messaging.Body;
import com.twilio.twiml.messaging.Message;

import static spark.Spark.*;
//import static spark.Spark.get;
//import static spark.Spark.post;

public class SMSReceiver {
    public static void main(String[] args) {
        get("/", (req, res) -> "Hello Web");

        post("/sms", (req, res) -> {
        	// The incoming request is not used/read
        	// A response (res) is being built here:
            res.type("application/xml");
            Body body = new Body
                    .Builder("The Robots are coming! Head for the hills!")
                    .build();
            Message sms = new Message
                    .Builder()
                    .body(body)
                    .build();
            MessagingResponse twiml = new MessagingResponse
                    .Builder()
                    .message(sms)
                    .build();
            return twiml.toXml();
        });
    }
}
