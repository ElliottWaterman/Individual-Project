package server;

import java.util.ArrayList;

/**
 * Class to define the generation of an HTML webpage for displaying report data.
 * @author Elliott Waterman
 */
public class ReportGenerator {
	/**
	 * Constant to define a table row open tag in HTML.
	 */
	private static final String TABLE_ROW_OPEN = "<tr>";
	/**
	 * Constant to define a table row close tag in HTML.
	 */
	private static final String TABLE_ROW_CLOSE = "</tr>";
	/**
	 * Constant to define a table column open tag in HTML.
	 */
	private static final String TABLE_COLUMN_OPEN = "<td>";
	/**
	 * Constant to define a table column close tag in HTML.
	 */
	private static final String TABLE_COLUMN_CLOSE = "</td>";
	
	/**
	 * Function to generate an HTML string containing data from arduino in 
	 * the format of a responsive table/report.
	 * @param listOfMessages A list containing data from the storage file.
	 * @return An HTML string containing a table of report data.
	 */
	public static String generateHTML(ArrayList<ArduinoMessage> listOfMessages) {
		// Create HTML document type and header
    	StringBuilder HTMLReport = new StringBuilder("<!doctype html><html lang=\"en\">" +
    		"<head>" +
    		  "<meta charset=\"utf-8\">" +
			  "<title>Smart Boa Snake Basking Station Data Report</title>" +
			  "<meta name=\"description\" content=\"Data report utilising SMS received from the smart basking station\">" +
			  "<meta name=\"author\" content=\"Elliott Waterman\">" +
			  "<!-- Favicon -->" +
			  "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"assets/favicon.ico\">" +
			  "<!-- jQuery include -->" +
			  "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>" +
			  "<!-- Tabulator include -->" +
			  "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/tabulator/4.1.4/css/tabulator.min.css\">" +
			  "<script type=\"text/javascript\" src=\"https://cdnjs.cloudflare.com/ajax/libs/tabulator/4.1.4/js/tabulator.min.js\"></script>" +
			  "<!-- Stylesheet -->" +
			  "<link rel=\"stylesheet\" href=\"assets/styles.css\">" +
			  "<!-- JavaScript -->" +
			  "<script type=\"text/javascript\" src=\"assets/main.js\"></script>" +
			"</head>" +
			"<body>");
    	
    	// Add title heading
    	HTMLReport.append("<h1>Smart Boa Snake Basking Station</h1>");
    	HTMLReport.append("<h2>Report Viewer</h2><br>");
    	
    	// Create table which is converted into a responsive table using Tabulator
    	// Add table header and tbody open tag
    	HTMLReport.append("<table id=\"report-table\"><thead><tr><th min-width=\"250\">ID</th><th>Phone Number</th>" +
    					  "<th>Time</th><th>RFID</th><th tabulator-align=\"center\">Temperature</th>" +
    					  "<th tabulator-align=\"center\">Weight</th></tr></thead><tbody>");
    	
		// Put each message data into an HTML column
		for (ArduinoMessage message : listOfMessages) {
			HTMLReport.append(TABLE_ROW_OPEN);
				HTMLReport.append(TABLE_COLUMN_OPEN);
					HTMLReport.append(message.getMessageSid());
				HTMLReport.append(TABLE_COLUMN_CLOSE);
				HTMLReport.append(TABLE_COLUMN_OPEN);
					HTMLReport.append(message.getPhoneNumber());
				HTMLReport.append(TABLE_COLUMN_CLOSE);
				HTMLReport.append(TABLE_COLUMN_OPEN);
					HTMLReport.append(message.getEpochMillis());
				HTMLReport.append(TABLE_COLUMN_CLOSE);
				HTMLReport.append(TABLE_COLUMN_OPEN);
					HTMLReport.append(message.getRFID());
				HTMLReport.append(TABLE_COLUMN_CLOSE);
				HTMLReport.append(TABLE_COLUMN_OPEN);
					HTMLReport.append(message.getTemperature());
				HTMLReport.append(TABLE_COLUMN_CLOSE);
				HTMLReport.append(TABLE_COLUMN_OPEN);
					HTMLReport.append(message.getWeight());
				HTMLReport.append(TABLE_COLUMN_CLOSE);
			HTMLReport.append(TABLE_ROW_CLOSE);
		}
		
		// Add tbody close tag and table close tag
		HTMLReport.append("</tbody></table>");
		
		// Add body end and HTML end
		HTMLReport.append("</body></html>");
    	
		// Return completed HTML string
    	return HTMLReport.toString();
	}
	
}	// End class ReportGenerator
