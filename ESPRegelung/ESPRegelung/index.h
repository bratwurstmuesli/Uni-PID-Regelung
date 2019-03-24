const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
	<HEAD>
		<TITLE>Wetterstation</TITLE>
	</HEAD>
<body>
<h1>ESP 8266 Wetterstation</h1>
<h2>Dataset:</h2>

Kp: <b><span id="Kp">0</span></b><br>
Ki: <b><span id="Ki">0</span></b><br>
Kd: <b><span id="Kd">0</span></b><br>
Setpoint: <b><span id="Setpoint">0</span></b><br>
Input: <b><span id="Input">0</span></b><br>
Output: <b><span id="Output">0</span></b><br>

<script>
setInterval(function() { getSetpoint();getInput();getOutput();getKp();getKi();getKd(); }, 100);
function getSetpoint() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Setpoint").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "Setpoint", true);
  xhttp.send();
}
function getKp() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Kp").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "Kp", true);
  xhttp.send();
}
function getKi() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Ki").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "Ki", true);
  xhttp.send();
}
function getpKd() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Kd").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "Kd", true);
  xhttp.send();
}
function getInput() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Input").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "Input", true);
  xhttp.send();
}
function getOutput() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Output").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "Output", true);
  xhttp.send();
}
</script>
<br><br><a href="https://thingspeak.com/channels/638623">Alle Werte Hier</a>
</body>
</html>
)=====";
