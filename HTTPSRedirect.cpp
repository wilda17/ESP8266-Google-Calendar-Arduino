/*  HTTPS with follow-redirect
 *  Created by Sujay S. Phadke, 2016
 *  All rights reserved.
 *  
 *  Modified by Daniel Willi, 2016
 *
 */

#include "HTTPSRedirect.h"

// from LarryD, Arduino forum
//#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

String data;

HTTPSRedirect::HTTPSRedirect(const int p, const char* fp, bool c) 
    : httpsPort(p), redirFingerprint(fp), fpCheck(c){
}

HTTPSRedirect::HTTPSRedirect(const int p) 
    : httpsPort(p){
      fpCheck = false;
}

HTTPSRedirect::~HTTPSRedirect(){ 
}

String HTTPSRedirect::getData(String& url, const char* host, const char* redirHost){
  return getData(url.c_str(), host, redirHost);
}

String HTTPSRedirect::getData(const char* url, const char* host, const char* redirHost){
  
  int redirFlag = false;

  // Check if connection to host is alive
  if (!connected()){
    Serial.println("Error! Not connected to host.");
    return "error";
  }
  // HTTP headers must be terminated with a "\r\n\r\n"
  // http://stackoverflow.com/questions/6686261/what-at-the-bare-minimum-is-required-for-an-http-request
  // http://serverfault.com/questions/163511/what-is-the-mandatory-information-a-http-request-header-must-contain
  String Request = createRequest(url, host);

  DPRINTLN(Request);
  // make request
  print(Request);
  String line;
  String redirUrl;
  
  DPRINTLN("Detecting re-direction.");
  DPRINTLN(redirHost);
  
  while (connected()) {
    line = readStringUntil('\n');
    DPRINTLN(line);
    if (line == "\r") {
      DPRINTLN("END OF HEADER");
      
      //DPRINTLN(line);
      break;
    }
    
    if (find("Location: ") ){
      find((char *)redirHost);
      DPRINTLN("Found re-direction URL!");
      redirUrl = readStringUntil('\n');
      redirFlag = true;
      break;
    }
    /*
    if (finder.findUntil("chunked", "\n\r") ){
      break;
    }*/
  }
  
  DPRINTLN("Body:\n");
  if (verboseInfo){
    fetchData(true, false);
  }
    
  else
    flush();
    
  if (!redirFlag){
    DPRINTLN("No re-direction URL found in header.");
    return "error";
  }
  //flush();
  //stop();

  DPRINTLN("Redirected URL:");
  DPRINTLN(redirUrl);
  
  Request = createRequest(redirUrl.c_str(), redirHost);
                          
  DPRINTLN("Connecting to:");
  DPRINTLN(redirHost);

  if (!connect(redirHost, httpsPort)) {
    Serial.println("Connection to re-directed host failed!");
    return "error";
  }

  if (fpCheck){
    if (verify(redirFingerprint, redirHost)) {
      Serial.println("Re-directed host certificate match.");
    } else {
      Serial.println("Re-directed host certificate mis-match");
    }
  }
  
  DPRINTLN("Requesting re-directed URL.");
  DPRINTLN(Request);

  // Make request
  print(Request);
  
  DPRINTLN("Final Response:");
  
  fetchData(false, true);

  fetchData(true, false);

  flush();
  //stop();

  return data;
}

String HTTPSRedirect::createRequest(const char* url, const char* host){
  return String("GET ") + url + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" +
                          "User-Agent: ESP8266\r\n" +
                          (keepAlive ? "" : "Connection: close") + 
                          "\r\n\r\n";
  
}

void HTTPSRedirect::fetchData(bool disp, bool header){
  String line;
  data = "";
  while (connected()) {
    line = readStringUntil('\n');
    
    if (disp){
      //Serial.println(line);  
      data += line + "|";   
    }
           
    if (line == "\r") {
      if (disp){
        if (header){
          DPRINTLN("END OF HEADER");
        }         
        else{
          DPRINTLN("END OF RESPONSE");
          DPRINTLN(line);
        }          
      }
      data.remove(data.length()-3);
      //Serial.println("break");
      break;
    }
  }
}


