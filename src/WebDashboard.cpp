#include "WebDashboard.h"

#include <ArduinoJson.h>
#include <WiFi.h>

#include "Config.h"

WebDashboard::WebDashboard(CameraService &camera, DetectionState &state, SemaphoreHandle_t stateMutex)
    : camera_(camera), state_(state), stateMutex_(stateMutex), server_(config::WEB_SERVER_PORT) {}

void WebDashboard::begin() {
  server_.on("/", HTTP_GET, [this]() { handleRoot(); });
  server_.on("/status", HTTP_GET, [this]() { handleStatus(); });
  server_.on("/stream", HTTP_GET, [this]() { handleStream(); });
  server_.onNotFound([this]() { server_.send(404, "text/plain", "Not found"); });

  server_.begin();
  startTask();

  Serial.printf("Web dashboard siap: http://%s/\n", WiFi.localIP().toString().c_str());
}

void WebDashboard::handleClient() {
  server_.handleClient();
}

bool WebDashboard::isStreamingActive() const {
  return activeStreamClients_ > 0;
}

void WebDashboard::startTask() {
  xTaskCreatePinnedToCore(taskEntry, "web-dashboard", 8192, this, 1, nullptr, 0);
}

void WebDashboard::taskEntry(void *parameter) {
  auto *dashboard = static_cast<WebDashboard *>(parameter);
  dashboard->taskLoop();
}

void WebDashboard::taskLoop() {
  for (;;) {
    handleClient();
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void WebDashboard::handleRoot() {
  server_.send(200, "text/html", buildHtmlPage());
}

void WebDashboard::handleStatus() {
  server_.send(200, "application/json", buildStatusJson());
}

void WebDashboard::handleStream() {
  WiFiClient client = server_.client();
  static const char *kBoundary = "frame";
  bool streamRegistered = false;

  Serial.println("Client stream terhubung");
  activeStreamClients_++;
  streamRegistered = true;

  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: multipart/x-mixed-replace; boundary=");
  client.print(kBoundary);
  client.print("\r\n");
  client.print("Cache-Control: no-cache, no-store, must-revalidate\r\n");
  client.print("Pragma: no-cache\r\n");
  client.print("Connection: close\r\n\r\n");

  while (client.connected()) {
    camera_fb_t *frame = nullptr;
    if (!camera_.capture(frame)) {
      Serial.println("Stream stop: gagal ambil frame");
      break;
    }

    client.print("--");
    client.print(kBoundary);
    client.print("\r\n");
    client.printf("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", static_cast<unsigned int>(frame->len));
    client.write(frame->buf, frame->len);
    client.printf("\r\n");

    camera_.release(frame);

    if (!client.connected()) {
      break;
    }

    client.flush();
    vTaskDelay(pdMS_TO_TICKS(40));
  }

  if (streamRegistered && activeStreamClients_ > 0) {
    activeStreamClients_--;
  }

  Serial.println("Client stream terputus");
}

String WebDashboard::buildStatusJson() {
  DetectionState snapshot;
  if (xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(200)) == pdTRUE) {
    snapshot = state_;
    xSemaphoreGive(stateMutex_);
  }

  JsonDocument doc;
  doc["label"] = snapshot.label;
  doc["confidence"] = snapshot.confidence;
  doc["detections"] = snapshot.detections;
  doc["latency_ms"] = snapshot.latencyMs;
  doc["valid"] = snapshot.valid;
  doc["updated_at"] = snapshot.updatedAt;

  String json;
  serializeJson(doc, json);
  return json;
}

String WebDashboard::buildHtmlPage() const {
  return R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Smart Waste Bin Live View</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #07111f;
      --panel: rgba(10, 21, 38, 0.86);
      --panel-2: rgba(15, 29, 51, 0.92);
      --text: #e8f1ff;
      --muted: #95a9c7;
      --accent: #5eead4;
      --border: rgba(148, 163, 184, 0.18);
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      font-family: "Segoe UI", "Trebuchet MS", sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at top left, rgba(94, 234, 212, 0.18), transparent 30%),
        radial-gradient(circle at top right, rgba(251, 191, 36, 0.14), transparent 25%),
        linear-gradient(180deg, #09111f 0%, #050814 100%);
    }
    .shell {
      width: min(1180px, calc(100% - 32px));
      margin: 0 auto;
      padding: 24px 0 32px;
    }
    .hero {
      display: grid;
      grid-template-columns: 1.4fr 0.9fr;
      gap: 20px;
      align-items: stretch;
    }
    .card {
      background: var(--panel);
      border: 1px solid var(--border);
      border-radius: 24px;
      box-shadow: 0 24px 80px rgba(0, 0, 0, 0.35);
      overflow: hidden;
      backdrop-filter: blur(18px);
    }
    .video-card { padding: 18px; }
    .title-row {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      align-items: center;
      margin-bottom: 16px;
    }
    h1 {
      margin: 0;
      font-size: clamp(1.6rem, 3vw, 2.4rem);
      letter-spacing: -0.03em;
    }
    .badge {
      padding: 10px 14px;
      border-radius: 999px;
      background: rgba(94, 234, 212, 0.12);
      color: var(--accent);
      border: 1px solid rgba(94, 234, 212, 0.28);
      font-size: 0.92rem;
      white-space: nowrap;
    }
    .stream-frame {
      aspect-ratio: 4 / 3;
      width: 100%;
      border-radius: 20px;
      background: #02050d;
      border: 1px solid rgba(148, 163, 184, 0.16);
      object-fit: cover;
    }
    .panel {
      display: grid;
      gap: 16px;
      padding: 18px;
      background: var(--panel-2);
    }
    .metric-grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
    }
    .metric {
      padding: 16px;
      border-radius: 18px;
      background: rgba(255, 255, 255, 0.03);
      border: 1px solid rgba(148, 163, 184, 0.12);
    }
    .metric span {
      display: block;
      color: var(--muted);
      font-size: 0.85rem;
      margin-bottom: 6px;
    }
    .metric strong { font-size: 1.2rem; }
    .status {
      padding: 18px;
      border-radius: 18px;
      background: linear-gradient(135deg, rgba(94, 234, 212, 0.12), rgba(251, 191, 36, 0.08));
      border: 1px solid rgba(148, 163, 184, 0.14);
    }
    .status h2 {
      margin: 0 0 12px;
      font-size: 1.05rem;
    }
    .status-value {
      font-size: clamp(1.6rem, 4vw, 2.6rem);
      font-weight: 700;
      line-height: 1;
      margin-bottom: 10px;
    }
    .muted { color: var(--muted); }
    .footer {
      margin-top: 16px;
      color: var(--muted);
      font-size: 0.92rem;
    }
    @media (max-width: 900px) {
      .hero { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <main class="shell">
    <section class="hero">
      <article class="card video-card">
        <div class="title-row">
          <div>
            <h1>Smart Waste Bin Live View</h1>
            <div class="muted">Realtime camera stream saat klasifikasi berjalan</div>
          </div>
          <div class="badge" id="connectionBadge">Connected</div>
        </div>
        <img class="stream-frame" src="/stream" alt="Camera stream" />
      </article>

      <aside class="card panel">
        <div class="status">
          <h2>Status Deteksi</h2>
          <div class="status-value" id="label">waiting</div>
          <div class="muted" id="updatedAt">Menunggu hasil terbaru...</div>
        </div>

        <div class="metric-grid">
          <div class="metric">
            <span>Confidence</span>
            <strong id="confidence">0%</strong>
          </div>
          <div class="metric">
            <span>Objects</span>
            <strong id="detections">0</strong>
          </div>
          <div class="metric">
            <span>Latency</span>
            <strong id="latency">0 ms</strong>
          </div>
          <div class="metric">
            <span>Valid</span>
            <strong id="valid">false</strong>
          </div>
        </div>

        <div class="footer">Refresh status otomatis setiap 1 detik.</div>
      </aside>
    </section>
  </main>

  <script>
    const labelEl = document.getElementById('label');
    const confidenceEl = document.getElementById('confidence');
    const detectionsEl = document.getElementById('detections');
    const latencyEl = document.getElementById('latency');
    const validEl = document.getElementById('valid');
    const updatedAtEl = document.getElementById('updatedAt');
    const badgeEl = document.getElementById('connectionBadge');

    async function refreshStatus() {
      try {
        const response = await fetch('/status', { cache: 'no-store' });
        const data = await response.json();

        labelEl.textContent = data.label || 'waiting';
        confidenceEl.textContent = `${Math.round((data.confidence || 0) * 100)}%`;
        detectionsEl.textContent = data.detections ?? 0;
        latencyEl.textContent = `${data.latency_ms ?? 0} ms`;
        validEl.textContent = String(Boolean(data.valid));
        updatedAtEl.textContent = data.valid ? `Updated at ${new Date().toLocaleTimeString()}` : 'Menunggu hasil terbaru...';
        badgeEl.textContent = 'Live';
      } catch (error) {
        badgeEl.textContent = 'Offline';
        updatedAtEl.textContent = 'Tidak bisa mengambil status dari device.';
      }
    }

    refreshStatus();
    setInterval(refreshStatus, 1000);
  </script>
</body>
</html>
)HTML";
}