const char* calibratePageHtml = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Калібрування ґрунтового датчика</title>
    <style>
      body { font-family: sans-serif; text-align: center; padding: 20px; }
      button { font-size: 1.2em; padding: 10px 20px; margin: 10px; }
    </style>
  </head>
  <body>
    <h2>Калібрування вологості ґрунту</h2>
    <p>Поточне значення з піну 33: <strong>%RAW%</strong></p>
    <p>Сухе значення (RAW_DRY): <strong>%DRY%</strong></p>
    <p>Вологе значення (RAW_WET): <strong>%WET%</strong></p>
    <form method="POST" action="/setDry">
      <button>Це повністю сухо</button>
    </form>
    <form method="POST" action="/setWet">
      <button>Це занурено у воду</button>
    </form>
  </body>
</html>
)rawliteral";
