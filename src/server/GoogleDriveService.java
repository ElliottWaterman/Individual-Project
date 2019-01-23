package server;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.GeneralSecurityException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import com.google.api.client.auth.oauth2.Credential;
import com.google.api.client.extensions.java6.auth.oauth2.AuthorizationCodeInstalledApp;
import com.google.api.client.extensions.jetty.auth.oauth2.LocalServerReceiver;
import com.google.api.client.googleapis.auth.oauth2.GoogleAuthorizationCodeFlow;
import com.google.api.client.googleapis.auth.oauth2.GoogleClientSecrets;
import com.google.api.client.googleapis.javanet.GoogleNetHttpTransport;
import com.google.api.client.http.FileContent;
import com.google.api.client.http.javanet.NetHttpTransport;
import com.google.api.client.json.JsonFactory;
import com.google.api.client.json.jackson2.JacksonFactory;
import com.google.api.client.util.store.FileDataStoreFactory;
import com.google.api.services.drive.Drive;
import com.google.api.services.drive.DriveScopes;
import com.google.api.services.drive.model.File;

public class GoogleDriveService {
    private static final String APPLICATION_NAME = "SBSBS Report Upload - Google Drive API Java";
    private static final JsonFactory JSON_FACTORY = JacksonFactory.getDefaultInstance();
    private static final String TOKENS_DIRECTORY_PATH = "tokens";
    // Delete previously saved tokens / folder if modifying the scopes
    // DRIVE gives the most permissions? Old: DRIVE_METADATA_READONLY
    private static final List<String> SCOPES = Collections.singletonList(DriveScopes.DRIVE);
    // Directory location of the credentials file
    private static final String CREDENTIALS_FILE_PATH = "assets/credentials.json";
    private static final String UPLOAD_REPORT_NAME = "SBSBS_Report_File";
    private static final String UPLOAD_REPORT_EXTENSION = ".csv";
    
    /**
     * Creates an authorised Credential object.
     * @param HTTP_TRANSPORT The network HTTP Transport.
     * @return An authorised Credential object.
     * @throws IOException If the credentials.json file cannot be found.
     */
    private static Credential getCredentials(final NetHttpTransport HTTP_TRANSPORT) throws IOException {
        // Load client secrets.
    	// GoogleDriveService.class.getResourceAsStream("/credentials.json"); // In the src folder
        InputStream in = Files.newInputStream(Paths.get(CREDENTIALS_FILE_PATH)); 
        GoogleClientSecrets clientSecrets = GoogleClientSecrets.load(JSON_FACTORY, new InputStreamReader(in));
        
        // Build flow and trigger user authorization request.
        GoogleAuthorizationCodeFlow flow = new GoogleAuthorizationCodeFlow.Builder(
                HTTP_TRANSPORT, JSON_FACTORY, clientSecrets, SCOPES)
                .setDataStoreFactory(new FileDataStoreFactory(new java.io.File(TOKENS_DIRECTORY_PATH)))
                .setAccessType("offline")
                .build();
        LocalServerReceiver receiver = new LocalServerReceiver.Builder().setPort(8888).build();
        return new AuthorizationCodeInstalledApp(flow, receiver).authorize("user");
    }
    
    /**
     * Function to upload the SBSBS CSV report file to Google Drive. May need authorising in default
     * browser before function completes.
     * @return The name of the uploaded file.
     * @throws IOException A thrown IO exception.
     * @throws GeneralSecurityException A general security exception.
     */
    public static String uploadReportFile() throws IOException, GeneralSecurityException {
        // Build a new authorized API client service.
        final NetHttpTransport HTTP_TRANSPORT = GoogleNetHttpTransport.newTrustedTransport();
        Drive driveService = new Drive.Builder(HTTP_TRANSPORT, JSON_FACTORY, getCredentials(HTTP_TRANSPORT))
                .setApplicationName(APPLICATION_NAME)
                .build();
        
        // Get current date/time and set into form "2019_01_25_23:59:59"
        String currentDateTime = new SimpleDateFormat("yyyy_MM_dd_HH:mm:ss").format(new Date());
        
        File fileMetadata = new File();
        fileMetadata.setName(UPLOAD_REPORT_NAME + currentDateTime + UPLOAD_REPORT_EXTENSION);
        // Relate/convert into Google spreadsheet
        //fileMetadata.setMimeType("application/vnd.google-apps.spreadsheet");
        
        java.io.File filePath = new java.io.File("SBSBS.csv");
        FileContent mediaContent = new FileContent("text/csv", filePath);
        File file = driveService.files().create(fileMetadata, mediaContent)
            .setFields("id")
            .execute();
        //System.out.println("File ID: " + file.getId());
        
        return fileMetadata.getName();
    }
}

