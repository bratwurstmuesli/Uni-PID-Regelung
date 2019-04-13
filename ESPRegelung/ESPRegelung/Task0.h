//this page is not being used somehow...

const char Websocket_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<body>
    <header>
      <h1>PID Regelung</h1>
    </header>
    <div>
      <table style="width: 100%;">
        <tr>
          <td style="width: 10%; text-align: right;">Kp: </td>
          <td style="width: 60%;"><input class="enabled" style="width: 100%;" id="Kp" type="range" min="0" max="5" step="0.01" oninput="sendPID();" value="1.30"></td>
		<td style="width: 20%;"><p id="Kpf">0</p></td>
        </tr>
        <tr>
          <td style="width: 10%; text-align: right;">Ki: </td>
          <td style="width: 60%;"><input class="enabled" style="width: 100%;" id="Ki" type="range" min="0" max="5" step="0.01" oninput="sendPID();" value="2.10"></td>
		<td style="width: 20%;"><p id="Kif">0</p></td>
        </tr>
        <tr>
          <td style="width: 10%; text-align: right;">Kd: </td>
          <td style="width: 60%;"><input class="enabled" style="width: 100%;" id="Kd" type="range" min="0" max="5" step="0.01" oninput="sendPID();" value="0.10"></td>
		  <td style="width: 20%;"><p id="Kdf">0</p></td>
        </tr>
          <tr>
          <td style="width: 10%; text-align: right;">Setpoint: </td>
          <td style="width: 60%;"><input class="enabled" style="width: 100%;" id="Setpoint" type="range" min="0" max="2500" step="10" oninput="sendPID();" value="1000"></td>
		  <td style="width: 20%;"><input type="text" style="width: 90%;" id="Setpointfield" onkeydown="if(event.keyCode == 13) sendPID1();"></td>
		  <td style="width: 10%;"><p style="text-align: left;" id="Setpointf">0</p></td>
        </tr>
		<tr>
          <td style="width: 10%; text-align: right;">Input: </td>
		<td style="width: 60%;"><p id="Inputf">0</p></td>
        </tr>
		<tr>
          <td style="width: 10%; text-align: right;">Output: </td>
		<td style="width: 60%;"><p id="Outputf">0</p></td>
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