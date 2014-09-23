import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

/**
 * 
 */

/**
 * @author mayanknarasimhan and narendran kp
 *
 */
public class webcrawler {

	/**
	 * @param args
	 */

	private static String setCookie = "";
	static Set<String> toBeVisited = new HashSet<String>();
	static Set<String> frontier = new HashSet<String>();
	static Set<String> flags = new HashSet<String>();
	private static int NUM_SECRET_FLAGS = 5;
	static int linksParsed = 0;

	public static void main(String[] args) {

		String username = "001914701";
		String password = "JDPLUP9I";
		/* 
		FLAG: 968a8cb41b03e0f2e5172feadf5241492265ad50f5c9d7231d31b7e8e49fdb16
		FLAG: 7000cb3b56c65604f8dda2d601cafeb5bdbaee11df87acfd1a469aca2f700918
		FLAG: a726924f75b63592653f791be7842c26c6cf5c578b889f0e9c3492aa037b8c05
		FLAG: 9a607414862a3389c7fc2c2a3caf4a935167fe05e6389b9692d57f3a182d83e2
		FLAG: f497b2c6c146303b73c76f77555c2e0e369c0d6ca154478e4834ea04bac46782 
		
		FLAG: d51aab53b5f199dd708b204ff8addd48694086dca7a9a8b482cedd205fbcaeec
		FLAG: 91c10b6a918ab97ce9eca3a97365e4b7aac40d0f863a17e2587d38235df2d04c
		FLAG: d51aab53b5f199dd708b204ff8addd48694086dca7a9a8b482cedd205fbcaeec
		FLAG: d51aab53b5f199dd708b204ff8addd48694086dca7a9a8b482cedd205fbcaeec
		FLAG: 4afc885ac06870677d7d992ef494be426ab339ac92f553a54fa27a1e224df072
		FLAG: 735a754efdd5714ff95357b0618a5a69b5ef73bcf4f37f83655de451fa930bb4

		*/
		String loginPage = "/accounts/login/";
		
		if (args.length == 2) {
			username = args[0];
			password = args[1];
		}
		else {
			System.out.println("Insufficient Arguments provided");
			System.exit(0);
		}

		try {

			/* Get the login page from the server */
			String response = getRequest(loginPage);

			String responseHTML = getResponseHTML(response);

			/* Find the cookie and hidden token from the server response */
			setCookie = getCookies(response);

			/* Find the hidden csrf token from the embedded html body */
			String csrf = getCsrfToken(responseHTML);

			/* Authenticate with the server through a POST request */
			response = postRequest(loginPage, username, password, csrf, setCookie);

			/* Crawl pages and find secret flags */
			responseHTML = getResponseHTML(response);
			Document doc = Jsoup.parse(responseHTML);
			if(isErrorLogin(doc)) {
				System.out.println("Incorrect username/password entered");
				System.exit(1);
			}
				
			parseDocument(doc);
			toBeVisited.addAll(frontier);
			while(!toBeVisited.isEmpty()) {
				crawlPages();
				if(flags.size() >= NUM_SECRET_FLAGS) {
					Iterator<String> flagsIterator = flags.iterator();
					while(flagsIterator.hasNext()) {
						System.out.println(flagsIterator.next().toString().trim());
					}
					break;
				}
			}
			//System.out.println(linksParsed);

		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private static int getHTTPStatus (String response) {
		Pattern p = Pattern.compile("(?s)(?i)HTTP/1.1 (.*?)\\n");
		Matcher m = p.matcher(response.toString());
		String httpCode = "";
		int statusCode;
		if(m.find(0)) {
			httpCode = m.group(1);
		}
		statusCode = Integer.parseInt(httpCode.split(" ")[0]);
		return statusCode;
	}

	private static String getResponseHTML (String response) {
		Pattern p = Pattern.compile("(?s)(?i)<html.*?>(.*?)</html>");
		Matcher m = p.matcher(response.toString());
		String responseHTML = "";
		if(m.find(0)) {
			responseHTML = m.group(0);
		}
		return responseHTML;
	}

	private static String getCookies(String response) {
		String cookie[] = new String[10];
		String cookieName[] = new String[10];
		String cookieValue[] = new String[10];
		String cookies[] = new String[10];
		String sendingCookie = "";

		/* Find Cookie in header received from server */
		Pattern p = Pattern.compile("(?s)Set-Cookie: (.*?)\\n");
		Matcher m = p.matcher(response);
		int i = 0;
		while(m.find()) {
			cookies[i] = m.group(0);
			i++;
		}

		/* Get cookie in name value pairs */
		i = 0;
		if(cookies.length != 0) {
			for (String c : cookies) {
				if(c != null && !c.isEmpty()) {
					cookie[i] = c.substring(c.indexOf(" "), c.indexOf(";"));
					cookieName[i] = cookie[i].substring(0, cookie[i].indexOf("="));
					cookieValue[i] = cookie[i].substring(cookie[i].indexOf("=")+1, cookie[i].length());
					i++;
				}
			}

			/* Join name value pairs into single string */
			for (i=0; i<cookieName.length; i++) {
				if (cookieName[i] != null && !cookieName[i].isEmpty())
					sendingCookie = sendingCookie + cookieName[i].trim() + "=" + cookieValue[i].trim() + "; ";
				if (i == cookieName.length - 1)
					sendingCookie = sendingCookie.substring(0, sendingCookie.length()-2);
			}
		}
		return sendingCookie;
	}

	private static String getRequest(String path) throws IOException {
		String host = "cs5700.ccs.neu.edu";
		int port = 80;
		int contentLength = -1;
		String redirectLocation = "";

		String getRequest = "GET " + path + " HTTP/1.1\n";
		getRequest = getRequest + "Host: " + host + ":" + port + "\n";
		if(!setCookie.isEmpty())
			getRequest = getRequest + "Cookie: " + setCookie + "\n";
		getRequest = getRequest + "From: mayankn@ccs.neu.edu\n";
		getRequest = getRequest + "Connection: Keep-Alive\n";
		getRequest = getRequest + "User-Agent: HTTPWebCrawler/1.0\n\n";

		/* Write the GET request to a socket and send it to the server */
		Socket socket = new Socket(InetAddress.getByName(host), port);
		OutputStream os = socket.getOutputStream();
		os.write(getRequest.getBytes());
		os.flush();
		
		/* Receive the server response */
		BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		String line = "";
		String headers = "";
		String responseBody = "";

		while ((line = in.readLine()) != null) {
			/* Reading HTTP header */
			headers = headers + line + "\n";
			if (line.isEmpty()) 
				/* No more headers to read */ 
				break;
		}

		/* Check HTTP response status code */
		int httpStatus = getHTTPStatus(headers);

		/* Make decision as to what to do for each HTTP status code */
		switch(httpStatus) {
		case 200: /* All OK */
			contentLength = getContentLength(headers);

			/* Content-Length present in header */
			if(contentLength != -1) {
				responseBody = readFullContent(in, contentLength);
			}

			/* Transfer-Encoding: chunked sent by the server */
			else {
				responseBody = readChunkedContent(in);
			}
			break;
		case 301: /* Page moved permanently or redirect requested to another page */
			redirectLocation = getRedirectLocation(headers);

			/* Remove the URL prefix including the http:// and hostname from the path  */
			redirectLocation = "/" + removeURLPrefix(redirectLocation);
			return getRequest(redirectLocation);
		case 302: /* Page moved temporarily or redirect requested to another page */
			redirectLocation = getRedirectLocation(headers);

			/* Remove the URL prefix including the http:// and hostname from the path  */
			redirectLocation = "/" + removeURLPrefix(redirectLocation);
			return getRequest(redirectLocation);
		case 403: /* URL Forbidden */
			return "";
		case 404: /* URL not found */
			return "";
		default: /* No action for un-handled status codes */
			return "";
		}

		//System.out.println(headers+responseBody	);
		linksParsed++;
		os.close();
		in.close();
		socket.close();
		return headers + responseBody;
	}

	private static String postRequest(String path, String username, String password, String csrf, String cookie) throws IOException {
		String host = "cs5700.ccs.neu.edu";
		int port = 80;
		int contentLength = -1;
		String redirectLocation = "";

		/* Construct the POST request to the server  */
		String postBody = "csrfmiddlewaretoken=" + csrf + "&" + "username=" + username + "&" + "password=" + password + "&" + "next=/fakebook/";
		//String postBody = "csrfmiddlewaretoken=" + csrf + "&" + "username=" + username + "&" + "password=" + password;// + "&" + "next=/fakebook/";
		String postRequest = "POST /accounts/login/ HTTP/1.1\n";
		postRequest = postRequest + "Host: " + host + "\n";
		postRequest = postRequest + "Cookie: " + cookie.trim() + "\n";
		postRequest = postRequest + "User-Agent: HTTPWebCrawler/1.0\n";
		postRequest = postRequest + "Connection: Keep-Alive\n";
		postRequest = postRequest + "Content-Type: application/x-www-form-urlencoded\n";
		postRequest = postRequest + "Content-Length: " + postBody.length() + "\r\n\r\n";
		postRequest = postRequest + postBody + "\r\n"; 

		/* Write the GET request to a socket and send it to the server */
		Socket socket = new Socket(InetAddress.getByName(host), port);
		OutputStream os = socket.getOutputStream();
		os.write(postRequest.getBytes());
		os.flush();
		
		/* Receive the server response */
		BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		String line = "";
		String headers = "";
		String responseBody = "";

		while ((line = in.readLine()) != null) {
			/* Reading HTTP header */
			headers = headers + line + "\n";

			if (line.isEmpty()) 
				/* No more headers to read */
				break;
		}

		/* Check HTTP response status code */
		int httpStatus = getHTTPStatus(headers);

		/* Make decision as to what to do for each HTTP status code */
		switch(httpStatus) {
		case 200: /* All OK */
			/* Set the new cookie sent by the server */
			setCookie = getCookies(headers);

			contentLength = getContentLength(headers);

			/* Content-Length present in header */
			if(contentLength != -1) {
				responseBody = readFullContent(in, contentLength);
			}

			/* Transfer-Encoding: chunked sent by the server */
			else {
				responseBody = readChunkedContent(in);
			}
			break;
		case 301: /* Page moved permanently or redirect requested to another page */
			/* Set the new cookie sent by the server */
			setCookie = getCookies(headers);

			redirectLocation = getRedirectLocation(headers);

			/* Remove the URL prefix including the http:// and hostname from the path  */
			redirectLocation = "/" + removeURLPrefix(redirectLocation);
			return getRequest(redirectLocation);
		case 302: /* Page moved temporarily or redirect requested to another page */
			/* Set the new cookie sent by the server */
			setCookie = getCookies(headers);

			redirectLocation = getRedirectLocation(headers);

			/* Remove the URL prefix including the http:// and hostname from the path  */
			redirectLocation = "/" + removeURLPrefix(redirectLocation);
			return getRequest(redirectLocation);
		case 403: /* URL Forbidden */
			return null;
		case 404: /* URL not found */
			return null;
		default: /* No action for unhandled status codes */
			return null;
		}
		
		os.close();
		in.close();
		socket.close();
		return headers + responseBody;
	}

	private static int getContentLength(String headers) {
		int contentLength = -1;
		Pattern p = Pattern.compile("(?s)Content-Length: (.*?)\\n");
		Matcher m = p.matcher(headers);
		if(m.find()) {
			contentLength = Integer.parseInt(m.group(1));
		}

		return contentLength;
	}

	private static String readFullContent(BufferedReader in, int contentLength) throws IOException {
		String responseBody = "";
		int count = 0;
		while (count < contentLength) {
			/* Reading remaining HTTP response body */
			responseBody = responseBody + ((char)in.read());
			count ++;
		}

		return responseBody;
	}

	private static String readChunkedContent(BufferedReader in) throws IOException {
		String responseBody = "";
		int count = 0;
		int chunkLength = -1;
		while(chunkLength != 0) {
			if ((chunkLength = getChunkLength(in)) != -1) {
				count = 0;
				while (count < chunkLength) {
					/* Reading remaining HTTP response body */
					responseBody = responseBody + ((char)in.read());
					count ++;
				}
			}
		}
		return responseBody;
	}

	private static int getChunkLength(BufferedReader in) throws IOException {
		int chunkLength = -1;
		String r = "";

		r = in.readLine();
		if(r != null && r.isEmpty()) {
			r = in.readLine();
		}

		chunkLength = Integer.parseInt(r, 16);

		return chunkLength;
	}

	private static String getRedirectLocation(String headers) {
		String location = "";
		Pattern p = Pattern.compile("(?s)Location: (.*?)\\n");
		Matcher m = p.matcher(headers);
		if(m.find()) {
			location = m.group(1);
		}

		return location;
	}

	private static String getCsrfToken(String responseHTML) {
		String csrf = "";

		if (!responseHTML.isEmpty()) {

			/* Parse html content and find the element that contains the hidden token */
			Document doc = Jsoup.parse(responseHTML);
			Element csrfInput = doc.select("input[name=csrfmiddlewaretoken]").first();
			csrf = csrfInput.attr("value");
		}

		return csrf;
	}

	private static String removeURLPrefix(String path) {
		Pattern p = Pattern.compile("(http|https)://(.*?)/");
		Matcher m = p.matcher(path);
		if(m.find()) {
			path = path.replaceAll(m.group(0), "").trim();
		}

		return path;
	}

	private static void parseDocument(Document doc) {
		Element body = doc.body();
		Elements links = body.getElementsByTag("a");
		for (Element link : links) {
			frontier.add(link.attr("href"));
		}
	}

	private static void crawlPages() throws IOException {
		//Boolean flag = false;
		String responseHTML = "";
		Iterator<String> linksIterator = toBeVisited.iterator();
		while(linksIterator.hasNext()) {
			String temp =linksIterator.next();
			//System.out.println(temp);
			String response = getRequest(temp.trim());
			linksIterator.remove();
			responseHTML = getResponseHTML(response);
			findSecretFlag(responseHTML);
			Document doc = Jsoup.parse(responseHTML);
			parseDocument(doc);
		}
		toBeVisited.addAll(frontier);
	}

	private static void findSecretFlag(String responseHTML) {
		Document doc = Jsoup.parse(responseHTML);
		Element body = doc.body();
		Elements sFlags = body.getElementsByClass("secret_flag");
		for (Element sFlag : sFlags) {
			flags.add(sFlag.text().substring(5, sFlag.text().length()));
		}
	}
	
	private static Boolean isErrorLogin(Document doc) {
		Element body = doc.body();
		Elements errorList = body.getElementsByClass("errorlist");
		for (Element error : errorList) {
			if(error.text().contains("Please enter a correct username and password. Note that both fields are case-sensitive.")) {
				return true;
			}
		}
		
		return false;
	}

}
