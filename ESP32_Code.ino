#include <WiFi.h>

const char* ssid = "iG";     // 🔁 Replace with your WiFi SSID
const char* password = "p";  // 🔁 Replace with your WiFi password

WiFiServer server(80);
const int ECG_PIN = 32;

void setup() {
  Serial.begin(115200);
  pinMode(ECG_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    String req = client.readStringUntil('\r');
    client.read(); // Clear buffer

    if (req.indexOf("GET /ecg") >= 0) {
      // ✅ Smooth signal using a 5-sample average
      int sum = 0;
      for (int i = 0; i < 5; i++) {
        sum += analogRead(ECG_PIN);
        delay(1); // Small delay for sampling
      }

      float avg = sum / 5.0;

      // ✅ Normalize signal to 0.0–1.0
      float normalized = avg / 4095.0;

      // ✅ Center signal around 0 (-0.5 to +0.5)
      float centered = normalized - 0.5;

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Access-Control-Allow-Origin: *"); // ✅ Allow cross-origin
      client.println("Connection: close");
      client.println();
      client.println(centered, 5);  // Send value with 5 decimal places
    } else {
      // ✅ Serve a minimal webpage
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ECG</title>");
      client.println("<script>");
      client.println("setInterval(() => {");
      client.println("  fetch('/ecg').then(r => r.text()).then(val => {");
      client.println("    document.getElementById('val').innerText = val;");
      client.println("  });");
      client.println("}, 200);");
      client.println("</script></head><body>");
      client.println("<h2>💓 Real-Time ECG Reading</h2>");
      client.println("<div id='val' style='font-size:2em;color:#007ACC;'>Loading...</div>");
      client.println("</body></html>");
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}
