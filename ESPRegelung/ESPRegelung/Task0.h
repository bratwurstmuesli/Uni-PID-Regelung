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
		<td><p id="Kpf">0</p></td>
        </tr>
        <tr>
          <td style="width:40px; text-align: right">Ki: </td>
          <td><input class="enabled" id="Ki" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		<td><p id="Kif">0</p></td>
        </tr>
        <tr>
          <td style="width:40px; text-align: right">Kd: </td>
          <td><input class="enabled" id="Kd" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		  <td><p id="Kdf">0</p></td>
        </tr>
          <tr>
          <td style="width:40px; text-align: right">Setpoint: </td>
          <td><input class="enabled" id="Setpoint" type="range" min="0" max="2000" step="10" oninput="sendPID();" value="0"></td>
		  <td><input type="text" id="Setpointfield" onkeydown="if(event.keyCode == 13) sendPID1();"></td>
		  <td><p id="Setpointf">0</p></td>
        </tr>
		<tr>
          <td style="width:40px; text-align: right">Input: </td>
		<td><p id="Inputf">0</p></td>
        </tr>
		<tr>
          <td style="width:40px; text-align: right">Output: </td>
		<td><p id="Outputf">0</p></td>
        </tr>
      </table>
    </div>
</body>

<script>
var Socket;
function start() {
	Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

	Socket.onmessage = function(evt) {
	console.log('Server: ' + evt.data);
	var server_message = evt.data;
	var server_message_char = server_message.charAt(0);

	if(server_message_char == "D"){
	var splitString = server_message.split(",");
	document.getElementById("Kpf").innerHTML = splitString[1];
	document.getElementById("Kif").innerHTML = splitString[2];
	document.getElementById("Kdf").innerHTML = splitString[3];
	document.getElementById("Setpointf").innerHTML = splitString[4];
	
	}

	else if(server_message_char == "R"){
	var splitString1 = server_message.split(",");
	document.getElementById("Inputf").innerHTML = splitString1[1];
	document.getElementById("Outputf").innerHTML = splitString1[2];
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

function sendPID1 () {
	var Kp = document.getElementById("Kp").value;
	var Ki = document.getElementById('Ki').value;
	var Kd = document.getElementById('Kd').value;
	var Setpoint = document.getElementById("Setpointfield").value;
	document.getElementById("Setpoint").value = Setpoint;
	var pidstr = '#' + Kp.toString() + ',' + Ki.toString() + ',' + Kd.toString() + ',' + Setpoint.toString() + ',';
	Socket.send(pidstr);
}

</script>

<body onload = "javascript:start();">
</body>

</html>
)=====";