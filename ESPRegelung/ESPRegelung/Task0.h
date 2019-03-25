//this page is not being used somehow...

const char Websocket_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<body>
  <center>
    <header>
      <h1>LED Control</h1>
    </header>
    <div>
      <table>
        <tr>
          <td style="width:40px; text-align: right">Kp: </td>
          <td><input class="enabled" id="Kp" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		<td><textarea class="rxd" id="Kpf"  resize="none" readonly></textarea></td>
        </tr>
        <tr>
          <td style="width:40px; text-align: right">Ki: </td>
          <td><input class="enabled" id="Ki" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		<td><textarea class="rxd" id="Kif"  resize="none" readonly></textarea></td>
        </tr>
        <tr>
          <td style="width:40px; text-align: right">Kd: </td>
          <td><input class="enabled" id="Kd" type="range" min="0" max="10" step="1" oninput="sendPID();" value="0"></td>
		  <td><label class="rxd" id="Kdf">0</label></textarea></td>
        </tr>
      </table>
    </div>
  </center>
</body>

<script>
var Socket;
function start() {
	Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

	Socket.onmessage = function(evt) {
	document.getElementById("Kdf").value = evt.data;
  }
}

function sendPID () {
	var Kp = document.getElementById("Kp").value;
	var Ki = document.getElementById('Ki').value;
	var Kd = document.getElementById('Kd').value;
	var pidstr = '#' + Kp.toString() + ',' + Ki.toString() + ',' + Kd.toString() + ',';
	Socket.send(pidstr);
}

</script>

<body onload = "javascript:start();">
</body>

</html>
)=====";