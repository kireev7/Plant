#ifndef WIFI_HPP
#define WIFI_HPP

const char* wifiPageHtml = R"rawliteral(
<!DOCTYPE html>
<html lang='uk'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Налаштування Wi-Fi</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 50px; }
    h1 { color: #333; }
    form { display: inline-block; text-align: left; }
    label { display: block; margin: 10px 0 5px; }
    input { width: 100%; padding: 8px; margin-bottom: 10px; }
    .button-container {
      display: flex;
      justify-content: space-between;
      margin-top: 10px;
    }
    button, a.button { 
      padding: 10px 20px; 
      background-color: #4CAF50; 
      color: white; 
      border: none; 
      cursor: pointer; 
      text-decoration: none; 
      display: inline-block; 
      margin: 5px 0;
      border-radius: 8px;
      box-shadow: 0 2px 5px rgba(0,0,0,0.2);
    }
    button:hover, a.button:hover { background-color: #45a049; }
  </style>
</head>
<body>
  <h1>Налаштування Wi-Fi</h1>
  <form action='/setWifi' method='POST'>
    <label for='ssid'>SSID:</label>
    <input type='text' id='ssid' name='ssid' maxlength='31' required>
    <label for='password'>Пароль:</label>
    <input type='password' id='password' name='password' maxlength='63'>
    <div class="button-container">
      <button type='submit'>Зберегти</button>
      <a href="/" class="button">Назад</a>
    </div>
  </form>
  <div style="margin-top: 20px;">
    <a href="/resetWifi" class="button">Скинути налаштування Wi-Fi</a>
  </div>
</body>
</html>
)rawliteral";

#endif