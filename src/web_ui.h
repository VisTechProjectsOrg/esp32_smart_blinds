#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Blinds</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: #1a1a2e; color: #eee; padding: 16px;
    max-width: 480px; margin: 0 auto;
  }
  h1 { font-size: 1.4em; text-align: center; margin-bottom: 8px; color: #e94560; }
  .status { text-align: center; font-size: 0.85em; color: #888; margin-bottom: 20px; }
  .card {
    background: #16213e; border-radius: 12px; padding: 20px;
    margin-bottom: 16px;
  }
  .card h2 { font-size: 1em; margin-bottom: 12px; color: #e94560; }
  .position-display {
    text-align: center; font-size: 3em; font-weight: bold;
    margin: 10px 0; color: #0f3460;
  }
  .position-display span { color: #e94560; }
  .btn-row { display: flex; gap: 10px; margin-bottom: 12px; }
  .btn {
    flex: 1; padding: 14px; border: none; border-radius: 8px;
    font-size: 1em; font-weight: bold; cursor: pointer;
    transition: opacity 0.2s; color: #fff;
  }
  .btn:active { opacity: 0.7; }
  .btn-open { background: #0f3460; }
  .btn-close { background: #533483; }
  .btn-stop { background: #e94560; }
  .btn-jog { background: #0f3460; min-height: 60px; font-size: 1.3em; }
  .btn-cal { background: #533483; }
  .btn-cal.set { background: #1a8a4a; }
  .btn-small { padding: 10px; font-size: 0.9em; }
  .btn:disabled { opacity: 0.4; cursor: not-allowed; }
  .slider-wrap { margin: 12px 0; }
  .slider-wrap input[type=range] {
    width: 100%; height: 8px; -webkit-appearance: none;
    background: #0f3460; border-radius: 4px; outline: none;
  }
  .slider-wrap input[type=range]::-webkit-slider-thumb {
    -webkit-appearance: none; width: 28px; height: 28px;
    background: #e94560; border-radius: 50%; cursor: pointer;
  }
  .slider-label { display: flex; justify-content: space-between; font-size: 0.8em; color: #888; }
  .collapsible { cursor: pointer; user-select: none; }
  .collapsible::after { content: ' +'; }
  .collapsible.active::after { content: ' -'; }
  .collapse-content { display: none; margin-top: 12px; }
  .collapse-content.show { display: block; }
  .form-row { margin-bottom: 10px; }
  .form-row label { display: block; font-size: 0.85em; color: #888; margin-bottom: 4px; }
  .form-row input, .form-row select {
    width: 100%; padding: 8px; border: 1px solid #333;
    border-radius: 6px; background: #1a1a2e; color: #eee; font-size: 0.95em;
  }
  .form-row input[type=checkbox] { width: auto; margin-right: 8px; }
  .check-row { display: flex; align-items: center; }
  .msg { text-align: center; padding: 8px; font-size: 0.85em; color: #1a8a4a; }
  .msg.warn { color: #e94560; }
  .not-calibrated {
    background: #e94560; color: #fff; text-align: center;
    padding: 12px; border-radius: 8px; margin-bottom: 16px; font-weight: bold;
  }
</style>
</head>
<body>

<h1 id="title">Smart Blinds</h1>
<div class="status" id="statusBar">Connecting...</div>

<div id="calWarning" class="not-calibrated" style="display:none;">
  Not calibrated - open Calibration below to set up
</div>

<!-- Main Controls -->
<div class="card">
  <div class="position-display"><span id="posVal">--</span>%</div>
  <div class="slider-wrap">
    <input type="range" id="posSlider" min="0" max="100" value="0">
    <div class="slider-label"><span>Closed</span><span>Open</span></div>
  </div>
  <div class="btn-row">
    <button class="btn btn-close" onclick="sendCmd('close')">Close</button>
    <button class="btn btn-stop" onclick="sendCmd('stop')">Stop</button>
    <button class="btn btn-open" onclick="sendCmd('open')">Open</button>
  </div>
</div>

<!-- Calibration -->
<div class="card">
  <h2 class="collapsible" onclick="toggle(this)">Calibration</h2>
  <div class="collapse-content">
    <p style="font-size:0.85em; color:#888; margin-bottom:12px;">
      Use the jog buttons to move the blinds. Set the closed and open positions.
    </p>
    <div class="btn-row">
      <button class="btn btn-jog" id="jogUpBtn">Jog Up</button>
      <button class="btn btn-jog" id="jogDownBtn">Jog Down</button>
    </div>
    <div class="btn-row">
      <button class="btn btn-cal set btn-small" onclick="sendCmd('setClosed')">Set as CLOSED</button>
      <button class="btn btn-cal set btn-small" onclick="sendCmd('setOpen')">Set as OPEN</button>
    </div>
    <div class="msg" id="calMsg"></div>
  </div>
</div>

<!-- Schedule -->
<div class="card">
  <h2 class="collapsible" onclick="toggle(this)">Sunrise / Sunset Schedule</h2>
  <div class="collapse-content">
    <div class="form-row">
      <div class="check-row">
        <input type="checkbox" id="autoOpen" checked>
        <label for="autoOpen">Auto-open at sunrise</label>
      </div>
    </div>
    <div class="form-row">
      <label>Sunrise offset (minutes, +/- )</label>
      <input type="number" id="sunriseOff" value="0" min="-120" max="120">
    </div>
    <div class="form-row">
      <div class="check-row">
        <input type="checkbox" id="autoClose" checked>
        <label for="autoClose">Auto-close at sunset</label>
      </div>
    </div>
    <div class="form-row">
      <label>Sunset offset (minutes, +/- )</label>
      <input type="number" id="sunsetOff" value="0" min="-120" max="120">
    </div>
    <div class="form-row">
      <label>Sunrise / Sunset today</label>
      <div style="font-size:0.9em; color:#ccc;" id="sunTimes">--</div>
    </div>
    <button class="btn btn-cal btn-small" style="width:100%;" onclick="saveSchedule()">Save Schedule</button>
    <div class="msg" id="schedMsg"></div>
  </div>
</div>

<script>
let pollTimer;

function sendCmd(cmd) {
  fetch('/api/' + cmd, {method:'POST'}).then(r => r.json()).then(update);
}

function update(data) {
  if (!data) return;
  document.getElementById('posVal').textContent = data.position;
  document.getElementById('posSlider').value = data.position;
  document.getElementById('statusBar').textContent =
    (data.moving ? 'Moving... ' : '') +
    'IP: ' + data.ip + ' | ' + data.name;
  document.getElementById('calWarning').style.display = data.calibrated ? 'none' : 'block';
  if (data.sunrise) {
    document.getElementById('sunTimes').textContent = data.sunrise + ' / ' + data.sunset;
  }
}

function poll() {
  fetch('/api/status').then(r => r.json()).then(update).catch(() => {});
}

// Slider - send on release
let sliderTimeout;
document.getElementById('posSlider').addEventListener('change', function() {
  fetch('/api/move', {
    method: 'POST',
    headers: {'Content-Type':'application/json'},
    body: JSON.stringify({position: parseInt(this.value)})
  }).then(r => r.json()).then(update);
});
// Live update display while dragging
document.getElementById('posSlider').addEventListener('input', function() {
  document.getElementById('posVal').textContent = this.value;
});

// Jog buttons - hold to move, release to stop
function setupJog(btnId, direction) {
  const btn = document.getElementById(btnId);
  let active = false;
  function start(e) {
    e.preventDefault();
    if (active) return;
    active = true;
    fetch('/api/jog', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({direction: direction})
    });
  }
  function stop(e) {
    e.preventDefault();
    if (!active) return;
    active = false;
    fetch('/api/jogStop', {method:'POST'});
  }
  btn.addEventListener('mousedown', start);
  btn.addEventListener('mouseup', stop);
  btn.addEventListener('mouseleave', stop);
  btn.addEventListener('touchstart', start, {passive:false});
  btn.addEventListener('touchend', stop, {passive:false});
  btn.addEventListener('touchcancel', stop);
}
setupJog('jogUpBtn', 1);
setupJog('jogDownBtn', -1);

// Collapsible sections
function toggle(el) {
  el.classList.toggle('active');
  el.nextElementSibling.classList.toggle('show');
}

// Schedule
function saveSchedule() {
  const data = {
    autoOpen: document.getElementById('autoOpen').checked,
    autoClose: document.getElementById('autoClose').checked,
    sunriseOffset: parseInt(document.getElementById('sunriseOff').value) || 0,
    sunsetOffset: parseInt(document.getElementById('sunsetOff').value) || 0
  };
  fetch('/api/schedule', {
    method: 'POST',
    headers: {'Content-Type':'application/json'},
    body: JSON.stringify(data)
  }).then(r => r.json()).then(d => {
    document.getElementById('schedMsg').textContent = 'Saved!';
    setTimeout(() => document.getElementById('schedMsg').textContent = '', 2000);
  });
}

// Load schedule settings on page load
fetch('/api/schedule').then(r => r.json()).then(d => {
  if (d) {
    document.getElementById('autoOpen').checked = d.autoOpen;
    document.getElementById('autoClose').checked = d.autoClose;
    document.getElementById('sunriseOff').value = d.sunriseOffset;
    document.getElementById('sunsetOff').value = d.sunsetOffset;
  }
});

// Start polling
poll();
pollTimer = setInterval(poll, 2000);
</script>
</body>
</html>
)rawliteral";
