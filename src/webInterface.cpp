
#include <webInterface.h>

// WiFi Server
const char* ssid = "ESP32-Screen";
const char* password = "123456789";
const char* host = "esp32sd";
const char* webFolder = "";
WebServer server(80);

int test;

bool hasSD = false;
File uploadFile;

void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSdCard(String path) {
  String dataType = "text/plain";
  path = webFolder + path;
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  
  if (path.endsWith(".src")) {
    path = path.substring(0, path.lastIndexOf("."));
  } else if (path.endsWith(".htm")) {
    dataType = "text/html";
  } else if (path.endsWith(".css")) {
    dataType = "text/css";
  } else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (path.endsWith(".png")) {
    dataType = "image/png";
  } else if (path.endsWith(".gif")) {
    dataType = "image/gif";
  } else if (path.endsWith(".jpg")) {
    dataType = "image/jpeg";
  } else if (path.endsWith(".ico")) {
    dataType = "image/x-icon";
  } else if (path.endsWith(".xml")) {
    dataType = "text/xml";
  } else if (path.endsWith(".pdf")) {
    dataType = "application/pdf";
  } else if (path.endsWith(".zip")) {
    dataType = "application/zip";
  }

  File dataFile = SD.open(path.c_str());
  if (dataFile.isDirectory()) {
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }
  printf("Loading file: %s\n", path.c_str());
  if (!dataFile) {
    printf("Failed (T_T)\n");
    return false;
  }

  if (server.hasArg("download")) {
    dataType = "application/octet-stream";
  }

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    printf("Sent less data than expected!\n");
  }

  dataFile.close();
  return true;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (SD.exists((char *)upload.filename.c_str())) {
      SD.remove((char *)upload.filename.c_str());
    }
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    printf("Upload: START, filename: %s\n", upload.filename.c_str());
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
    printf("\rUpload: WRITE, Bytes: %d\n", upload.totalSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    printf("\nUpload: END, Size: %d\n", upload.totalSize);
  }
}

void deleteRecursive(String path) {
  printf("Deleting directory: %s\n", path.c_str());
  File file = SD.open((char *)path.c_str());
  if (!file.isDirectory()) {
    file.close();
    SD.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while (true) {
    File entry = file.openNextFile();
    if (!entry) {
      break;
    }
    String entryPath = entry.name();
    if (entry.isDirectory()) {
      entry.close();
      deleteRecursive(entryPath);
    } else {
      entry.close();
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD.rmdir((char *)path.c_str());
  file.close();
}

void handleDelete() {
  printf("Start Delete\n");
  if (server.args() == 0) {
    return returnFail("BAD ARGS");
  }
  String path = server.arg(0);
  if (path == "/" || !SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  deleteRecursive(path);
  returnOK();
}

void handleCreate() {
  printf("Start create\n");
  if (server.args() == 0) {
    return returnFail("BAD ARGS");
  }
  String path = server.arg(0);
  if (path == "/" || SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }

  if (path.indexOf('.') > 0) {
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if (file) {
      file.write(0);
      file.close();
    }
  } else {
    SD.mkdir((char *)path.c_str());
  }
  returnOK();
}

void printDirectory() {
  printf("Printing directory: ");
  if (!server.hasArg("dir")) {
    return returnFail("BAD ARGS");
  }
  String path = server.arg("dir");
  printf("%s\n", path.c_str());
  if (path != "/" && !SD.exists((char *)path.c_str())) {
    printf("BAD PATH\n");
    return returnFail("BAD PATH");
  }
  File dir = SD.open((char *)path.c_str());
  path = String();
  if (!dir.isDirectory()) {
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  //WiFiClient client = server.client(); //????????????????

  server.sendContent("[");
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }

    String output;
    if (cnt > 0) {
      output = ',';
    }

    path = entry.name();
    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += path.substring(path.lastIndexOf("/")+1);
    output += "\"";
    output += "}";
    printf("%s\n", output.c_str());
    server.sendContent(output);
    entry.close();
  }
  server.sendContent("]");
  dir.close();
}

void handleNotFound() {
  if (hasSD) {
    if(loadFromSdCard(server.uri()))
    {
      return;
    }
    else
    {
      printf("Failed loading file\n");
    }
  }
  else
  {
    printf("SDCARD Not Detected\n\n");
  }
  

  String message = "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  printf("%s", message.c_str());
}

void setupServer()
{
    //tft.println("Setting AP (Access Point)...");
    printf("Setting AP (Access Point)…\n");
    WiFi.softAP(ssid, password);

    //tft.print("AP IP address: ");
    printf("AP IP address: \n");
    //tft.println(WiFi.softAPIP());
    printf("%s\n", WiFi.softAPIP().toString().c_str());

    if (MDNS.begin(host)) {
        MDNS.addService("http", "tcp", 80);
        printf("MDNS responder started\n");
        printf("You can now connect to http://%s.local/\n", host);
    }

    server.on("/list", HTTP_GET, printDirectory);
    server.on("/edit", HTTP_DELETE, handleDelete);
    server.on("/edit", HTTP_PUT, handleCreate);
    server.on("/edit", HTTP_POST, returnOK, handleFileUpload);
    server.onNotFound(handleNotFound);

    server.begin();
    //tft.println("HTTP server started");
    printf("HTTP server started\n");
}

bool connectWiFi(const char* ssid_l, const char* pass)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_l, pass);
  printf("Connecting to %s\n", ssid_l);

  // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 40) {//wait 20 seconds
    delay(500);
    printf(".");
  }

  if (i >= 21) {
    printf("\nCould not connect to %s\n", ssid_l);
    return false;
  }
  printf("\nConnected! IP address: %s\n", WiFi.localIP().toString().c_str());
  return true;
}
