/**
 * 
 */

import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.SSLSocket;

import java.net.ConnectException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.io.*;
import java.net.UnknownHostException;

/**
 * @author Mayank Narasimhan and Narendran KP
 * 
 * This class is client program which implements TCP sockets to communicate with a remote server
 * and ultimately retrieves a secret message from the server at the end of the communication
 * 
 */
public class client {

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		String host = "";
		int port = 27993;
		boolean secureFlag = false;
		String NUID = "";

		try {
			
			/*
			 * Start processing the arguments only if the minimum number of required arguments are present
			 * Then parse the arguments and extract the values 
			 */
			if (args.length >= 2) {
				if (args[0].equals("-p")) {
					port = Integer.parseInt(args[1]);
					if (args[2].equals("-s")) {
						secureFlag = true;
						host = args[3];
						NUID = args[4];
					}
					else {
						host = args[2];
						NUID = args[3];
					}
				}
				else if (args[0].equals("-s"))   {
					port = 27994;
					secureFlag = true;
					host = args[1];
					NUID = args[2];
				}
				else {
					host = args[0];
					NUID = args[1];
				}
			}
			else {
				System.out.println("Insufficient Arguments provided");
				System.exit(0);
			}


			String constantMsg = "cs5700fall2013";
			
			/* The format of the HELLO message to be sent to initiate communication with the remote server */
			String helloMsg = constantMsg + " HELLO " + NUID + "\n";
			
			/* Converting the Host name or IP address notation given in the command line to an InetAddress
			 * This will ensure that the program understands either the IP address notation or the hostname notation
			 * and then connect to it using a TCP socket 
			 */
			InetAddress iaddr = InetAddress.getByName(host);
			
			/* Creating Secure Socket connection (-s parameter) */
			if (secureFlag) {
				
				/* Setting the path where the Java Trust Store will be present */
				System.setProperty("javax.net.ssl.trustStore", "jssecacerts");
				
				/* Creating an SSL Factory to generate SSL sockets */
				SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
				
				/* Creating an SSL socket using the SSL socket Factory 
				 * and setting the timeout for read() operations on this socket to 60 seconds
				 */
				SSLSocket sslsocket = (SSLSocket) factory.createSocket(iaddr, port);
				sslsocket.setSoTimeout(60000);
				
				/* Starting the handshake with the server */
				sslsocket.startHandshake();
				
				/* Creating suitable input and output streams to read and write to the server */
				OutputStream outputStream = sslsocket.getOutputStream();
				BufferedReader inputStream= new BufferedReader(new InputStreamReader(sslsocket.getInputStream()));

				// System.out.println("Client: " + helloMsg);
				
				outputStream.write(helloMsg.getBytes());

				String serverResponse = "";
				
				/* While there is some input to be read from the server */
				while((serverResponse = inputStream.readLine())!=null) {

					// System.out.println("Server: " + serverResponse);
					
					String[] responses = serverResponse.split(" ");

					/* 
					 * Split the server's response into individual strings to help in parsing the response
					 * Keep executing till the server sends a BYE message
					 */
					while (responses.length != 0 && !responses[2].trim().equals("BYE")) {
						if (responses[1].trim().equals("STATUS")) {
							char operator = responses[3].charAt(0);
							int soltn = 0;
							
							/* Perform the suitable arithmetic operation based on the operator */ 
							if (!Character.isWhitespace(operator)) {
								switch (operator) {
								case '+': soltn = (Integer.parseInt(responses[2]) + Integer.parseInt(responses[4]));
								break;
								case '-': soltn = (Integer.parseInt(responses[2]) - Integer.parseInt(responses[4]));
								break;
								case '*': soltn = (Integer.parseInt(responses[2]) * Integer.parseInt(responses[4]));
								break;
								case '/': soltn = (Integer.parseInt(responses[2]) / Integer.parseInt(responses[4]));
								break;
								default: System.out.println("Invalid Arithmetic Operator");
								break;
								}
								String soltnMsg = constantMsg + " " + soltn + "\n";
								
								// System.out.println("Client: " + soltnMsg);
								
								outputStream.write(soltnMsg.getBytes());
							}
						}
						serverResponse=inputStream.readLine();
						
						// System.out.println("Server: " + serverResponse);
						
						responses = serverResponse.split(" ");
					}

					/* Exit if unknown Husky ID is provided */
					if (!(responses[1].trim().equals("Unknown_Husky_ID"))) {
						System.out.println(responses[1]);
						break;
					}
					else {
						System.out.println("Client: Error - Unknown Husky ID provided");
						break;
					}

				}

				outputStream.close();
				inputStream.close();
				sslsocket.close();
			}
			
			
			/* Creating Normal Socket connection (no -s parameter) */
			else {
				
				/* Creating a normal socket connection
				 * and setting the timeout for read() operations on this socket to 60 seconds
				 */
				Socket socket = new Socket(iaddr, port);
				socket.setSoTimeout(60000);
				
				/* Creating suitable input and output streams to read and write to the server */
				OutputStream outputStream = socket.getOutputStream();
				BufferedReader inputStream= new BufferedReader(new InputStreamReader(socket.getInputStream()));
				
				// System.out.println("Client: " + helloMsg);
				
				outputStream.write(helloMsg.getBytes());

				String serverResponse = "";
				
				/* While there is some input to be read from the server */
				while((serverResponse = inputStream.readLine())!=null) {
					
					// System.out.println("Server: " + serverResponse);
					
					String[] responses = serverResponse.split(" ");

					/* Split the server's response into individual strings to help in parsing the response
					 * Keep executing till the server sends a BYE message
					 */
					while (responses.length != 0 && !responses[2].trim().equals("BYE")) {
						if (responses[1].trim().equals("STATUS")) {
							char operator = responses[3].charAt(0);
							int soltn = 0;
							
							/* Perform the suitable arithmetic operation based on the operator */
							if (!Character.isWhitespace(operator)) {
								switch (operator) {
								case '+': soltn = (Integer.parseInt(responses[2]) + Integer.parseInt(responses[4]));
								break;
								case '-': soltn = (Integer.parseInt(responses[2]) - Integer.parseInt(responses[4]));
								break;
								case '*': soltn = (Integer.parseInt(responses[2]) * Integer.parseInt(responses[4]));
								break;
								case '/': soltn = (Integer.parseInt(responses[2]) / Integer.parseInt(responses[4]));
								break;
								default: System.out.println("Invalid Arithmetic Operator");
								break;
								}
								String soltnMsg = constantMsg + " " + soltn + "\n";
								
								// System.out.println("Client: " + soltnMsg);
								
								outputStream.write(soltnMsg.getBytes());
							}
						}
						serverResponse=inputStream.readLine();
						
						// System.out.println("Server: " + serverResponse);
						
						responses = serverResponse.split(" ");
					}

					/* Exit if unknown Husky ID is provided */
					if (!(responses[1].trim().equals("Unknown_Husky_ID"))) {
						System.out.println(responses[1]);
						break;
					}
					else {
						System.out.println("Client: Error - Unknown Husky ID provided");
						break;
					}
				}

				outputStream.close();
				inputStream.close();
				socket.close();
			}
		}
		catch (UnknownHostException e) {
			System.out.println(e.getMessage());
		}
		catch (ConnectException e) {
			System.out.println(e.getMessage());
			System.out.println("The time out occured because the server is not responding on the specified port");
		}
		catch (SocketTimeoutException e) {
			System.out.println(e.getMessage());
			System.out.println("The socket timed out because a read() operation took more than 60 seconds or the server is not responding on the specified port");
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		catch (NumberFormatException e) {
			System.out.println(e.getMessage());
		}
	}
}