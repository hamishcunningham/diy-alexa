// httpd.cpp
// derived from ESPAsyncWebServer/examples/simple_server

#include <freertos/FreeRTOS.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

// debug macros
#define dbg(b, s) if(b) Serial.print(s)
#define dln(b, s) if(b) Serial.println(s)
#define startupDBG      true
#define loopDBG         true
#define monitorDBG      true
#define netDBG          true
#define miscDBG         true
#define analogDBG       true
#define otaDBG          true

// mega-simple templating
typedef struct { int position; const char *replacement; } replacement_t;
void getHtml(String& html, const char *[], int, replacement_t [], int);
#define ALEN(a) ((int) (sizeof(a) / sizeof(a[0]))) // only in definition scope!
#define GET_HTML(strout, boiler, repls) \
  getHtml(strout, boiler, ALEN(boiler), repls, ALEN(repls));

// templating: turn array of strings & set of replacements into a String
void getHtml(
  String& html, const char *boiler[], int boilerLen,
  replacement_t repls[], int replsLen
) {
  for(int i = 0, j = 0; i < boilerLen; i++) {
    if(j < replsLen && repls[j].position == i)
      html.concat(repls[j++].replacement);
    else
      html.concat(boiler[i]);
  }
}

// a page boilerplate: constants & pattern parts of template
const char *boiler[] = {
  "<html><head><title>                                                \n", // 00
  "christmas colours                                                  \n", // 01
  "</title>                                                           \n", // 02
  "<meta charset='utf-8'>                                             \n", // 03
  "<meta name='viewport'                                              \n", // 04
  "  content='width=device-width, initial-scale=1.0'>                 \n", // 05
  "<style>                                                            \n", // 06
  "body, p, label, input { font: 1rem 'Fira Sans', sans-serif;        \n", // 07
  "  background:#FFF; color: #000; font-size: 150%; }                 \n", // 08
  "input { margin: .4rem; }                                           \n", // 09
  "form.form-example { display: table; }                              \n", // 10
  "div.form-example { display: table-row; }                           \n", // 11
  "div.submit-form { display: center; }                               \n", // 12
  "label, input { display: table-cell; }                              \n", // 13
  "</style>                                                           \n", // 14
  "</head><body>                                                      \n", // 15
  "<form action='' method='get' class='form-example'>                 \n", // 16
  "<!--                                                               \n", // 17
  "  <p>style:</p>                                                    \n", // 18
  "  <div class='form-example'>                                       \n", // 19
  "    <label for='xmas'>xmas</label>                                 \n", // 20
  "    <input type='radio' id='xmas' name='runner' value='xmas'       \n", // 21
  "      checked>                                                     \n", // 22
  "  </div>                                                           \n", // 23
  "  <div class='form-example'>                                       \n", // 24
  "    <label for='off'>off</label>                                   \n", // 25
  "    <input type='radio' id='off' name='runner' value='off'>        \n", // 26
  "  </div>                                                           \n", // 27
  "  <br/>                                                            \n", // 28
  "  <p>colours:</p>                                                  \n", // 29
  "-->                                                                \n", // 30
  "  <div class='form-example'>                                       \n", // 31
  "    <label for='red'>red</label>                                   \n", // 32
  "    <input type='range' id='red' name='red' min='0' max='255'      \n", // 33
  "      value='127'>                                                 \n", // 34
  "  </div> <div class='form-example'>                                \n", // 35
  "    <label for='green'>green</label>                               \n", // 36
  "    <input type='range' id='green' name='green' min='0' max='255'  \n", // 37
  "      value='127'>                                                 \n", // 38
  "  </div> <div class='form-example'>                                \n", // 39
  "    <label for='blue'>blue</label>                                 \n", // 40
  "    <input type='range' id='blue' name='blue' min='0' max='255'    \n", // 41
  "      value='127'>                                                 \n", // 42
  "  </div>                                                           \n", // 43
  "  <br/><br/>                                                       \n", // 44
  "  </div> <div class='form-example'>                                \n", // 45
  "    <label for='bright'>brightness</label>                         \n", // 46
  "    <input type='range' id='bright' name='bright' min='0' max='100'\n", // 47
  "      value='10'>                                                  \n", // 48
  "  </div>                                                           \n", // 49
  "  <br/><br/>                                                       \n", // 50
  "  <div class='form-example'>                                       \n", // 51
  "    <label for='submit-button'>   </label>                         \n", // 52
  "    <input type='submit' value='update' id='submit-button'>        \n", // 53
  "  </div>                                                           \n", // 54
  "</form><pre>                                                       \n", // 55
  "<!-- req params -->                                                \n", // 56
  "</pre>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;               ", // 57
  "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(<a href='/'>reset</a>)        \n", // 58
  "</body></html>                                                     \n", // 59
  "                                                                   \n", // 60
};

/*
  for(int i = 0; i < ALEN(boiler); i++) // print the boilerplate for reference
    dbg(miscDBG, boiler[i]);

  getHtml(htmlPage, boiler, ALEN(boiler), repls, ALEN(repls)); // instantiate

  dbg(miscDBG, htmlPage.c_str()); // print the result
*/

AsyncWebServer server(80);
const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void initWebServer() {
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello, world");
  });
  */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    dln(netDBG, "serving page notionally at /");
    String paramStr =  ""; // collects all req parameters
// naughty: ideally the values should come out of default_conf...
    String brightVal = "value='10'>\n";
    String redVal =    "value='127'>\n";
    String greenVal =  "value='127'>\n";
    String blueVal =   "value='127'>\n";

    int numParams = request->params();
    if(numParams > 0) paramStr = "     (current: ";
    for(int i=0; i < numParams; i++) {
      AsyncWebParameter* p = request->getParam(i);

      paramStr += p->name().c_str();
      paramStr += ": ";
      paramStr += p->value().c_str();
      if(i < ( numParams - 1 ))
        paramStr += "; ";
      else
        paramStr += ")";
      if(i % 2 == 0) paramStr += "\n      ";

      // for each of bright/red/green/blue set the relevant parameter in repls
      // so that the served version of / includes them
      if(strcmp("bright", p->name().c_str()) == 0) {
        brightVal = "value='" + p->value() + "'>\n";
      } else if(strcmp("red", p->name().c_str()) == 0) {
        redVal = "value='" + p->value() + "'>\n";
      } else if(strcmp("green", p->name().c_str()) == 0) {
        greenVal = "value='" + p->value() + "'>\n";
      } else if(strcmp("blue", p->name().c_str()) == 0) {
        blueVal = "value='" + p->value() + "'>\n";
      }
    }
    Serial.printf("GET[%s]\n", paramStr.c_str());

    replacement_t repls[] = { // the elements to replace in the boilerplate
      { 34, redVal.c_str() },
      { 38, greenVal.c_str() },
      { 42, blueVal.c_str() },
      { 48, brightVal.c_str() },
      { 56, paramStr.c_str() },
    };
    String htmlPage = ""; // a String to hold the resultant page
    GET_HTML(htmlPage, boiler, repls);
    request->send(200, "text/html", htmlPage);
  });


  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String message;
    if(request->hasParam(PARAM_MESSAGE)) {
      message = request->getParam(PARAM_MESSAGE)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    String message;
    if(request->hasParam(PARAM_MESSAGE, true)) {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);

  server.begin();
}
