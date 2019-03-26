//this page is not being used somehow...

const char Websocket_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<body>
    <header>
      <h1>PID Regelung</h1>
    </header>
    <div>
      <table>
        <tr>
          <td style="width:40px; text-align: right">Kp: </td>
          <td><input class="enabled" id="Kp" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		<td><p id="Kpf"></p>0</label></td>
        </tr>
        <tr>
          <td style="width:40px; text-align: right">Ki: </td>
          <td><input class="enabled" id="Ki" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		<td><p id="Kif"></p>0</label></td>
        </tr>
        <tr>
          <td style="width:40px; text-align: right">Kd: </td>
          <td><input class="enabled" id="Kd" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		  <td><p id="Kdf"></p>0</td>
        </tr>
          <tr>
          <td style="width:40px; text-align: right">Setpoint: </td>
          <td><input class="enabled" id="Setpoint" type="range" min="0" max="10000" step="100" oninput="sendPID();" value="0"></td>
		  <td><input type="text" id="Setpointf" onkeydown="if(event.keyCode == 13) sendPID();">0</td>
        </tr>
		<tr>
          <td style="width:40px; text-align: right">Input: </td>
		<td><p id="Inputf"></p>0</td>
        </tr>
		<tr>
          <td style="width:40px; text-align: right">Output: </td>
		<td><p id="Outputf"></p>0</label></td>
        </tr>
      </table>
    </div>
</body>

<script>
var Socket;
function start() {
	Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

	Socket.onmessage = function(evt) {
	var evt1 = evt.charAt(0);

	if(evt1 == "D"){
	const splitString = evt.split(",");
	document.getElementById("Kpf").value = splitString[1];
	document.getElementById("Kif").value = splitString[2];
	document.getElementById("Kdf").value = splitString[3];
	document.getElementById("Setpointf").value = splitString[4];
	}

	else if(evt1 == "R"){
	const splitString = evt.split(",");
	document.getElementById("Inputf").value = splitString[1];
	document.getElementById("Outputf").value = splitString[2];
	}
	
  }
}

function sendPID () {
	var Kp = document.getElementById("Kp").value;
	var Ki = document.getElementById('Ki').value;
	var Kd = document.getElementById('Kd').value;
	var Setpoint = document.getElementById("Setpoint").value;
	var pidstr = '#' + Kp.toString() + ',' + Ki.toString() + ',' + Kd.toString() + ',' + Setpoint.toString() + ',';
	Socket.send(pidstr);
}

</script>

<body onload = "javascript:start();">
</body>

</html>
)=====";