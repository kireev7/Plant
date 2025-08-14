#ifndef SITE_HPP
#define SITE_HPP

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Sensor Monitor</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        text-align: center;
        margin-top: 40px;
        background-color: #f5f5f5;
      }
      .container {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 20px;
      }
      .block {
        background-color: white;
        padding: 20px 30px;
        border-radius: 15px;
        box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        width: 300px;
      }
      .row {
        display: flex;
        justify-content: space-between;
        margin: 10px 0;
        font-size: 18px;
      }
      h1 {
        margin-bottom: 30px;
      }
      #waterStatus {
        font-weight: bold;
        margin-top: 15px;
      }
      .editable {
        cursor: pointer;
        text-decoration: none;
        color: black;
      }
    </style>
  </head>
  <body>
    <h1>🌿 Система зрошення</h1>
    <div class="container">

      <!-- 1. Загальна інформація -->
      <div class="block">
        <div class="row"><span>🌡 Температура:</span> <span id="temp">--</span> °C</div>
        <div class="row"><span>💧 Вологість повітря:</span> <span id="hum">--</span> %</div>
        <div class="row"><span>🚰 Реле:</span> <span id="relay">--</span></div>
        <div class="row"><span>⚡ Струм:</span> <span id="current">--</span> мА</div>
        <div id="waterStatus">💧 Статус води: --</div>
      </div>

      <!-- 2. Рослина 1 -->
      <div class="block">
        <div class="row">
          <span class="editable" id="plantName0">🌱 Рослина 1:</span>
          <span id="ground0">--</span> %
        </div>
      </div>

      <!-- 3. Рослина 2 -->
      <div class="block">
        <div class="row">
          <span class="editable" id="plantName1">🌱 Рослина 2:</span>
          <span id="ground1">--</span> %
        </div>
      </div>

      <!-- 4. Рослина 3 -->
      <div class="block">
        <div class="row">
          <span class="editable" id="plantName2">🌱 Рослина 3:</span>
          <span id="ground2">--</span> %
        </div>
      </div>

      <!-- Кнопка переходу до калібрування -->
      <div style="margin-top: 30px;">
        <a href="/calibrate" style="
          display: inline-block;
          padding: 10px 20px;
          background-color: #4CAF50;
          color: white;
          text-decoration: none;
          border-radius: 8px;
          font-size: 16px;
          box-shadow: 0 2px 5px rgba(0,0,0,0.2);">
          ⚙️ Калібрування сенсорів
        </a>
      </div>
    </div>

    <script>
      // --- Обробка перейменування ---
      function setupEditableNames() {
        for (let i = 0; i < 3; i++) {
          const el = document.getElementById('plantName' + i);
          const saved = localStorage.getItem('plantName' + i);
          if (saved) el.textContent = saved;

          el.addEventListener('click', () => {
            const newName = prompt("Нова назва рослини:", el.textContent);
            if (newName) {
              el.textContent = newName;
              localStorage.setItem('plantName' + i, newName);
            }
          });
        }
      }

      // --- Дані від ESP ---
      function updateData() {
        fetch("/data")
          .then(res => res.json())
          .then(data => {
            document.getElementById("temp").textContent = data.temp;
            document.getElementById("hum").textContent = data.hum;
            document.getElementById("ground0").textContent = data.ground0;
            document.getElementById("ground1").textContent = data.ground1;
            document.getElementById("ground2").textContent = data.ground2;

            const relayText = data.relay === 1 ? "УВІМКНЕНО" : "ВИМКНЕНО";
            document.getElementById("relay").textContent = relayText;

            if ("current" in data) {
              document.getElementById("current").textContent = data.current + " мА";
            }

            const waterStatus = document.getElementById("waterStatus");
            if (data.noWater === true) {
              waterStatus.textContent = "🚱 Немає води!";
              waterStatus.style.color = "red";
            } else {
              waterStatus.textContent = "💧 Вода в нормі";
              waterStatus.style.color = "green";
            }
          });
      }

      setupEditableNames();
      setInterval(updateData, 2000);
    </script>
  </body>
</html>
)rawliteral";

#endif
