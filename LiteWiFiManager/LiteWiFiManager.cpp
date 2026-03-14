#include "LiteWiFiManager.h"

LiteWiFiManager::LiteWiFiManager(Print *logger) : _server(80) {
  (void)logger;  // logger kept for API compatibility; printing gated by DEBUG
}

#define WM_LOG(msg) DBG_LOG("*wm:", msg)
#define WM_LOGF(fmt, ...) DBG_LOGF("*wm:", fmt, ##__VA_ARGS__)

bool LiteWiFiManager::begin(const char *apSsid,
                            const char *apPassword,
                            unsigned long configPortalTimeoutMs,
                            bool forcePortal) {
  if (!forcePortal && connectWithStored()) {
    return true;
  }

  startPortal(apSsid, apPassword, configPortalTimeoutMs);

  // Blocking loop until credentials are saved or timeout is reached.
  while (_portalActive) {
    _dns.processNextRequest();
    _server.handleClient();

    if (_connectRequested) {
      _connectRequested = false;
      stopPortal();
      connectWithNew(_newSsid, _newPass);
    }

    if (_portalDeadline > 0 &&
        static_cast<long>(millis() - _portalDeadline) >= 0) {
      WM_LOG("Config portal timeout");
      stopPortal();
    }

    delay(10);
  }

  return isConnected();
}

bool LiteWiFiManager::connectWithStored(unsigned long connectTimeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin();  // reconnect using credentials already stored by the core
  WiFi.persistent(false);
  WM_LOG("Connecting using stored WiFi credentials");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < connectTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setAutoReconnect(true);
    WM_LOG("Connected with stored credentials");
    return true;
  }

  WM_LOGF("Stored credentials failed, status=%d\n", WiFi.status());
  return false;
}

bool LiteWiFiManager::connectWithNew(const String &ssid,
                                     const String &password,
                                     unsigned long connectTimeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  WiFi.persistent(true);  // write to native storage when beginning
  WiFi.begin(ssid.c_str(), password.c_str());
  WiFi.persistent(false);

  WM_LOGF("Connecting to %s\n", ssid.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < connectTimeoutMs) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setAutoReconnect(true);
    WM_LOG("Connected and saved");
    return true;
  }

  WM_LOG("Connection failed");
  return false;
}

void LiteWiFiManager::startPortal(const char *apSsid,
                                  const char *apPassword,
                                  unsigned long timeoutMs) {
  WiFi.mode(WIFI_AP_STA);
  bool apResult = WiFi.softAP(apSsid, apPassword);
  if (apResult) {
    WM_LOGF("AP started: %s\n", apSsid);
  } else {
    WM_LOG("Failed to start AP");
  }

  IPAddress apIp = WiFi.softAPIP();
  _dns.start(53, "*", apIp);

  _server.on("/", [this]() { handleRoot(); });
  _server.on("/save", [this]() { handleSave(); });
  _server.on("/scan", [this]() { handleScan(); });
  _server.onNotFound([this]() {
    String redirect = String("http://") + WiFi.softAPIP().toString();
    _server.sendHeader("Location", redirect, true);
    _server.send(302, "text/plain", "");
  });
  _server.begin();

  _portalActive = true;
  _portalDeadline = timeoutMs > 0 ? millis() + timeoutMs : 0;
}

void LiteWiFiManager::stopPortal() {
  if (_portalActive) {
    _dns.stop();
    _server.stop();
    WiFi.softAPdisconnect(true);
    _portalActive = false;
  }
}

void LiteWiFiManager::handleRoot() {
  static const char page[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>WiFi Setup</title>
  <style>
    :root{
      --bg:#0b1020;--card:#11182a;--line:#26324b;--txt:#e6e9f0;
      --muted:#9fb0cc;--accent:#4da3ff
    }
    *{box-sizing:border-box}
    body{
      margin:0;min-height:100vh;display:grid;place-items:start center;padding:18px 10px;
      font-family:Segoe UI,system-ui,-apple-system,sans-serif;color:var(--txt);
      background:
        radial-gradient(circle at 12% 16%,rgba(56,99,255,.2),transparent 26%),
        radial-gradient(circle at 86% 8%,rgba(0,200,255,.18),transparent 24%),
        var(--bg);
    }
    .card{
      width:min(620px,100%);padding:20px;border:1px solid #1f2940;border-radius:14px;
      background:var(--card);box-shadow:0 16px 40px rgba(0,0,0,.35)
    }
    h2{margin:0 0 8px}
    .sub{margin:0 0 12px;color:var(--muted);font-size:14px}
    label{display:block;margin:12px 0 6px;color:#b8c2d6;font-size:14px}
    .row,.pw{display:flex;gap:8px}
    select,input,button{
      width:100%;padding:11px;border-radius:10px;border:1px solid var(--line);
      background:#0f1524;color:var(--txt);font-size:15px
    }
    select:focus,input:focus{
      outline:none;border-color:var(--accent);box-shadow:0 0 0 3px rgba(77,163,255,.2)
    }
    .pw input{flex:1}
    #toggle{width:86px;background:#16233a;color:#7fb6ff;font-size:12px;font-weight:700}
    #scan{width:130px}
    #save{margin-top:14px;background:linear-gradient(135deg,#1f6feb,#52b6ff);border:none;font-weight:700}
    .status{margin-top:10px;font-size:13px;color:var(--muted)}
  </style>
</head>
<body>
  <div class="card">
    <h2>Wi-Fi Setup</h2>
    <p class="sub">Select SSID, enter password, save.</p>

    <form method="POST" action="/save">
      <label>SSID</label>
      <div class="row">
        <select name="ssid" id="ssid" required>
          <option value="">Scanning...</option>
        </select>
        <button type="button" id="scan">Rescan</button>
      </div>

      <label>Password</label>
      <div class="pw">
        <input name="pass" id="pass" type="password" maxlength="63">
        <button id="toggle" type="button">Show</button>
      </div>

      <button id="save" type="submit">Save and connect</button>
      <div id="st" class="status">Scan in progress...</div>
    </form>
  </div>

  <script>
    const ssidEl = document.getElementById('ssid');
    const passEl = document.getElementById('pass');
    const toggleEl = document.getElementById('toggle');
    const statusEl = document.getElementById('st');
    const scanEl = document.getElementById('scan');

    function renderNetworks(networks) {
      ssidEl.innerHTML = '';
      if (!networks.length) {
        ssidEl.innerHTML = '<option value="">No networks found</option>';
        return;
      }

      for (let i = 0; i < networks.length; i++) {
        const net = networks[i];
        const opt = document.createElement('option');
        opt.value = net.ssid;
        opt.textContent = net.ssid + ' (' + net.rssi + ' dBm)';
        if (!i) opt.selected = true;
        ssidEl.appendChild(opt);
      }
    }

    async function scan() {
      statusEl.textContent = 'Scanning...';
      scanEl.disabled = true;
      try {
        const res = await fetch('/scan');
        const data = await res.json();
        renderNetworks(data.networks || []);
        statusEl.textContent = 'Network ready.';
      } catch (_) {
        statusEl.textContent = 'Scan failed';
      }
      scanEl.disabled = false;
    }

    scanEl.onclick = scan;
    ssidEl.onchange = () => { statusEl.textContent = 'SSID selected.'; };
    toggleEl.onclick = () => {
      const hidden = passEl.type === 'password';
      passEl.type = hidden ? 'text' : 'password';
      toggleEl.textContent = hidden ? 'Hide' : 'Show';
    };
    scan();
  </script>
</body>
</html>
)HTML";
  _server.send_P(200, "text/html", page);
}

void LiteWiFiManager::handleSave() {
  if (!_server.hasArg("ssid")) {
    _server.send(400, "text/plain", "Missing ssid");
    return;
  }

  _newSsid = _server.arg("ssid");
  _newPass = _server.hasArg("pass") ? _server.arg("pass") : "";
  _newSsid.trim();
  _newPass.trim();

  if (_newSsid.isEmpty()) {
    _server.send(400, "text/plain", "SSID required");
    return;
  }

  _server.send(200, "text/plain",
               "Saved. Closing portal and connecting...");

  _connectRequested = true;
}

void LiteWiFiManager::handleScan() {
  int n = WiFi.scanNetworks();
  String json = "{\"networks\":[";
  for (int i = 0; i < n; i++) {
    if (i) json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]}";
  _server.send(200, "application/json", json);
}
