// joinme.cpp
// a derivative of the Lua Joinme wifi config utility that started in Jan 2015 here
// https://github.com/hamishcunningham/fishy-wifi/commit/33bb0f352a9172681afaf73ada29b332d69d2b28
// evolved here // https://github.com/hamishcunningham/fishy-wifi/tree/master/lua/joinme and here
// https://github.com/hamishcunningham/fishy-wifi/tree/master/ardesp/waterelf32 and here
// https://gitlab.com/hamishcunningham/unphone/-/blob/master/lib/unPhone_Library/joinmeWfmgr.cpp
// and now lives here, now based on based on https://github.com/tzapu/WiFiManager

#include "joinme.h"
#include <WiFiManager.h>

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param (for non blocking w params)
String getParam(String name);
void saveParamCallback();

bool joinmeManageWiFi(const char *apSSID, const char *apKey) {
  // uncomment for testing
  // wm.resetSettings(); // wipe settings

  int customFieldLength = 40; // add a custom input field

  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field
  // Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom
  // Field Placeholder\"");

  // test custom html input type(checkbox) new (&custom_field)
  // WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field
  // Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"
  // type=\"checkbox\""); // custom html type

  // test custom html(radio)
  const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom i/p
  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  //
  // menu tokens,
  // "wifi","wifinoscan","info","param","close","sep","erase","restart","exit"
  // (sep is seperator) (if param is in menu, params will not show up in wifi
  // page!) const char* menu[] =
  // {"wifi","info","param","sep","restart","exit"}; wm.setMenu(menu,6);
  std::vector<const char *> menu =
    {"wifi","info","param","sep","restart","exit"};
  wm.setMenu(menu);

  wm.setClass("invert"); // set dark theme

  wm.setConnectTimeout(15); // how long to try to connect before continuing
  wm.setConfigPortalTimeout(120); // auto close configportal after n seconds
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names
  // wm.setMinimumSignalQuality(20);  // min RSSI % to show in scans, null=8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  wm.setScanDispPerc(true);           // show RSSI as % not graph icons

  bool res = wm.autoConnect(apSSID, apKey); // start password protected AP

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  } else {
    //if you get here you have connected to the WiFi
/* TODO unnesc?    wm.stopConfigPortal();
    wm.stopWebPortal(); */
    Serial.println("connected...yeey :)");
  }

  return res;
}

// read parameter from server, for custom hmtl input
String getParam(String name) {
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback() {
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}






/* lua derivative version:

#include "joinme.h"
#include <WiFiClient.h>
#include <DNSServer.h>
#include <Update.h>
#include <ESPAsyncWebServer.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP_;
void handleOTAProgress(size_t, size_t); // progress tracker

// 307 is temporary redirect. if we used 301 we'd probably break the user's
// browser for sites they were captured from until they cleared their cache
int TEMPORARY_REDIRECT = 307;

void doRedirect(AsyncWebServerRequest* request) {
  Serial.printf(
    "joinme redirecting captured client to: %s\n",
    apIP_.toString().c_str()
  );
  auto response = request->beginResponse(TEMPORARY_REDIRECT,"text/plain","");
  response->addHeader("Location","http://"+apIP_.toString()+"/");
  request->send(response);
}

void handleL0(AsyncWebServerRequest* request) {
  doRedirect(request);
}
void handleL2(AsyncWebServerRequest* request) {
  doRedirect(request);
}
void handleALL(AsyncWebServerRequest* request) {
  doRedirect(request);
}

void joinmeDNSSetup(void* server_p, IPAddress apIP) {
  AsyncWebServer* server = (AsyncWebServer *) server_p;
  assert(server != NULL);
  apIP_ = apIP;
  Serial.printf(
    "joinme will direct captured clients to: %s\n",
    apIP_.toString().c_str()
  );
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP_);
  Serial.println("joinme captive dns server started");
  server->on("/generate_204", doRedirect); // android captive portal
  server->on("/L0", handleL0);
  server->on("/L2", handleL2);
  server->on("/ALL", handleALL);
  Serial.println("joinme http handlers added");
}

void joinmeTurn() {
  dnsServer.processNextRequest();
}

// OTA over-the-air update stuff ///////////////////////////////////////////
// if the repo is public set gitToken to "" (otherwise the API must be used
// and a valid personal access token supplied)
void joinmeOTAUpdate(
  int firmwareVersion, String gitProjID, String gitToken, String relPath
) {
  // materials for doing an HTTP GET on github from the firmware/ dir
  HTTPClient http;              // manage the HTTP request process
  http.setTimeout(20000);       // increase the timeout of a request
  http.setReuse(true);          // keep alive
  int respCode;    // the response code from the request (e.g. 404, 200, ...)
  int highestAvailableVersion = -1;  // version of latest firmware on server
  String repoPath;
  if(gitToken.length() == 0) {  // use raw
    relPath.replace("%2F", "/");
    repoPath = relPath;
  } else {                      // use API
    repoPath = "/repository/files/" + relPath;
  }

  // do a GET to read the version file from the cloud
  Serial.println("checking for firmware updates...");
  respCode = joinmeCloudGet(&http, gitProjID, gitToken, repoPath + "version");
  if(respCode == 200) // check response code (-ve on failure)
    highestAvailableVersion = atoi(http.getString().c_str());
  else
    Serial.printf("couldn't get version! rtn code: %d\n", respCode);
  http.end(); // free resources

  // do we know the latest version, and does the firmware need updating?
  if(respCode != 200) {
    Serial.printf("cannot update\n\n");
    return;
  } else if(firmwareVersion >= highestAvailableVersion) {
    Serial.printf("firmware is up to date\n\n");
    return;
  }

  // ok, we need to do a firmware update...
  Serial.printf(
    "upgrading firmware from version %d to version %d\n",
    firmwareVersion, highestAvailableVersion
  );

  // do a GET for the .bin
  String binName = String(highestAvailableVersion);
  binName += ".bin";
  respCode = joinmeCloudGet(&http, gitProjID, gitToken, repoPath + binName);
  int updateLength = http.getSize(); // if isn't big enough refuse to update
  if(respCode == 200) {              // check response code (-ve on failure)
    Serial.printf(".bin code/size: %d; %d\n\n", respCode, updateLength);
    if(updateLength < 174992) {      // the size of the Blink example sketch
      Serial.println("update size is too small! refusing to try OTA update");
      return;
    }
  } else {
    Serial.printf("failed to get a .bin! return code is: %d\n", respCode);
    http.end(); // free resources
    return;
  }

  // write the new version of the firmware to flash
  WiFiClient* stream = http.getStreamPtr();
  Update.onProgress(handleOTAProgress); // print out progress
  if(Update.begin(updateLength)) {
    Serial.printf("starting OTA, may take a minute or two...\n");
    size_t written; // how much has been written
    written = Update.writeStream(*stream);
    stream->flush();
    Serial.printf("update written\n");

    // if the written amount is the same length as or longer than the
    // original content length, then it should have been written to the
    // device successfully
    if (written >= updateLength) {
      Serial.println("written " + String(written) + " successfully");
    } else {
      Serial.println(
        "oops, written only " + String(written) + "/" + String(updateLength)
      );
    }

    // if the end function returns a successful boolean then the OTA has been
    // completed, otherwise output the error
    if(Update.end()) {
      Serial.printf("update done, now finishing...\n");
      if(Update.isFinished()) {
        Serial.printf("update successfully finished; rebooting...\n\n");
        ESP.restart();
      } else {
        Serial.printf("update didn't finish correctly :(\n");
      }
    } else {
      Serial.printf("an update error occurred, #: %d\n" + Update.getError());
    }
  } else {
    Serial.printf("not enough space to start OTA update :(\n");
  }
  http.end(); // free resources
}

// helper for downloading from cloud firmware server via HTTP GET
int joinmeCloudGet(
  HTTPClient *http, String gitProjID, String gitToken, String fileName
) {
  // build up URL from components; for example:
  // https://gitlab.com/api/v4/projects/_GITLAB_PROJ_ID/repository/files/\
  // examples%2FOTAThing%2Ffirmware%2F4.bin/raw?private_token=_GITLAB_TOKEN\
  // &ref=master

  // the gitlab root certificate (for HTTPS); see
  // https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/
  // for how to pick these up using firefox
  const char* gitlabRootCA =
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
    "-----END CERTIFICATE-----\n"
    ;

  // set up URL for download via either "raw" or the API with access tok
  String baseUrl = "https://gitlab.com";
  String url;
  if(gitToken == NULL || gitToken.length() == 0) { // use raw URL ////
    baseUrl += "/hamishcunningham/unphone/raw/master/";
    url = baseUrl + fileName;
  } else {                                         // use API ////////
    baseUrl += "/api/v4/projects/";
    url = baseUrl + gitProjID + fileName +
      "/raw?private_token=" + gitToken + "&ref=master";
  }

  // make GET request and return the response code
  Serial.printf("joinmeCloudGet, url = %s\n", url.c_str());
  http->begin(url, gitlabRootCA);
  http->addHeader("User-Agent", "ESP32");
  return http->GET();
}

// callback handler for tracking OTA progress
void handleOTAProgress(size_t done, size_t total) {
  float progress = (float) done / (float) total;
  // dbf(otaDBG, "OTA written %d of %d, progress = %f\n", done, total, progress);

  int barWidth = 70;
  Serial.printf("[");
  int pos = barWidth * progress;
  for(int i = 0; i < barWidth; ++i) {
    if(i < pos)
      Serial.printf("=");
    else if(i == pos)
      Serial.printf(">");
    else
      Serial.printf(" ");
  }
  Serial.printf(
    "] %d %%%c", int(progress * 100.0), (progress == 1.0) ? '\n' : '\r'
  );
  Serial.flush();
}
*/
