#pragma once

const char* calibratePageHtmlTemplate = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Калібрування ґрунтового сенсора</title>
  <meta charset="utf-8">
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    button {
      padding: 15px 30px;
      margin: 10px;
      font-size: 18px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 8px;
    }
    p {
      font-size: 20px;
    }
    a {
      display: inline-block;
      margin-top: 20px;
      color: #333;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h2>Калібрування ґрунтового сенсора</h2>
  <p>Поточне сире значення: <b>%RAW%</b></p>
  <form action='/setDry' method='POST'>
    <button type='submit'>🌵 Встановити як Сухий ґрунт</button>
  </form>
  <form action='/setWet' method='POST'>
    <button type='submit'>💧 Встановити як Вологий ґрунт</button>
  </form>
  <p><b>RAW_DRY = %DRY%</b><br><b>RAW_WET = %WET%</b></p>
  <a href="/">⬅ Назад</a>
</body>
</html>
)rawliteral";
