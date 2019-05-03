
const char Websocket_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<body>
    <header>
        <h1>PID Regelung</h1>
    </header>
    <p>Studienarbeit Drehzahlregelung eines Gleichstrommotors von Heiko Wehner</p>
    <div>
        <table style="width: 100%;">
            <tr>
               <!--<form>
                    <input id="PIDradio" type="radio" name="gender" value="R" onclick="myFunction()" checked> PID
                    <br>
                    <input id="PWMradio" type="radio" name="gender" value="S" onclick="myFunction()"> PWM
                    <br>
                </form>--><!-- das funktioniert nicht daher habe ich es kommentiert-->
            </tr>
        </table>
    </div>
    <div>
        <style>
            #myDIV {}
            
            #myDIV2 {}
        </style>
    </div>

</body>

<div id="myDIV">
    <table style="width: 100%;">
        <tr>
            <td style="width: 10%; text-align: right;">Kp: </td>
            <td style="width: 60%;">
                <input class="enabled" style="width: 100%;" id="Kp" type="range" min="0" max="5" step="0.01" oninput="sendPID();" value="1.30">
            </td>
            <td style="width: 20%;">
                <p id="Kpf">0</p>
            </td>
        </tr>
        <tr>
            <td style="width: 10%; text-align: right;">Ki: </td>
            <td style="width: 60%;">
                <input class="enabled" style="width: 100%;" id="Ki" type="range" min="0" max="5" step="0.01" oninput="sendPID();" value="2.10">
            </td>
            <td style="width: 20%;">
                <p id="Kif">0</p>
            </td>
        </tr>
        <tr>
            <td style="width: 10%; text-align: right;">Kd: </td>
            <td style="width: 60%;">
                <input class="enabled" style="width: 100%;" id="Kd" type="range" min="0" max="5" step="0.01" oninput="sendPID();" value="0.10">
            </td>
            <td style="width: 20%;">
                <p id="Kdf">0</p>
            </td>
        </tr>
        <tr>
            <td style="width: 10%; text-align: right;">Setpoint: </td>
            <td style="width: 60%;">
                <input class="enabled" style="width: 100%;" id="Setpoint" type="range" min="0" max="2500" step="10" oninput="sendPID();" value="1000">
            </td>
            <td style="width: 20%;">
                <input type="text" style="width: 90%;" id="Setpointfield" onkeydown="if(event.keyCode == 13) sendPID1();">
            </td>
            <td style="width: 10%;">
                <p style="text-align: left;" id="Setpointf">0</p>
            </td>
        </tr>
    </table>
</div>

<div hidden id="myDIV2">
    <table style="width: 100%;">
        <tr>
            <td style="width: 10%; text-align: right;">PWM: </td>
            <td style="width: 60%;">
                <input class="enabled" style="width: 100%;" id="PWMs" type="range" min="0" max="255" step="1" oninput="sendPWM();" value="150">
            </td>
            <td style="width: 20%;">
                <p id="PWMf">0</p>
            </td>
        </tr>
    </table>
</div>

<table style="width: 100%;">
    <tr>
        <td style="width: 10%; text-align: right;">Input: </td>
        <td style="width: 60%;">
            <p id="Inputf">0</p>
        </td>
    </tr>
    <tr>
        <td style="width: 10%; text-align: right;">Output: </td>
        <td style="width: 60%;">
            <p id="Outputf">0</p>
        </td>
    </tr>
</table>

<script>
    var Socket;
    var t0 = 0;
    var t1 = 0;
    var aktualisiert = 1;

    function start() {
        Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

        Socket.onmessage = function(evt) {
            console.log('Server: ' + evt.data);
            var server_message = evt.data;
            var server_message_char = server_message.charAt(0);

            if (server_message_char == "D") {
                console.log('Split variables D');
                var splitString = server_message.split(",");
                document.getElementById("Kpf").innerHTML = splitString[1];
                document.getElementById("Kif").innerHTML = splitString[2];
                document.getElementById("Kdf").innerHTML = splitString[3];
                document.getElementById("Setpointf").innerHTML = splitString[4];
                console.log('END Split variables D');
            } else if (server_message_char == "R") {
                console.log('Split variables start R');
                var splitString1 = server_message.split(",");
                console.log('Split variables R');
                document.getElementById("Inputf").innerHTML = splitString1[1];
                document.getElementById("Outputf").innerHTML = splitString1[2];
                console.log('Split variables end R');
            } else if (server_message_char == "P") {
                console.log('Split variables P');
                var splitString2 = server_message.split(",");
                document.getElementById("PWMf").innerHTML = splitString2[1];
            }
            aktualisiert = 1;
        }
    }

    function myFunction() {
        var x = document.getElementById("myDIV");
        var y = document.getElementById("myDIV2");
        if (x.style.display === "none") {
            x.style.display = "block";
            y.style.display = "none";
            var pidstr4 = 'S' + '1' + ',';
            Socket.send(pidstr4);
            console.log('changed Mode');

        } else {
            x.style.display = "none";
            y.style.display = "block";
            var pidstr4 = 'S' + '2' + ',' + '2' + ',';
            Socket.send(pidstr4);
            console.log('changed Mode');
        }
    }

    function sendPID() {
        t0 = performance.now();
        var letzersend = t0 - t1;
        console.log("letzersend ist: " + letzersend);
        if (letzersend >= 50 || aktualisiert == 1) {
            var Kp = document.getElementById("Kp").value;
            var Ki = document.getElementById('Ki').value;
            var Kd = document.getElementById('Kd').value;
            var Setpoint = document.getElementById("Setpoint").value;
            var pidstr = '#' + Kp.toString() + ',' + Ki.toString() + ',' + Kd.toString() + ',' + Setpoint.toString() + ',';
            Socket.send(pidstr);
            t1 = performance.now();
            aktualisiert = 0;
        }
    }

    function sendPID1() {
        t0 = performance.now();
        var letzersend = t0 - t1;
        console.log("letzersend ist: " + letzersend);
        if (letzersend >= 50 || aktualisiert == 1) {
            var Kp = document.getElementById("Kp").value;
            var Ki = document.getElementById('Ki').value;
            var Kd = document.getElementById('Kd').value;
            var Setpoint = document.getElementById("Setpointfield").value;
            document.getElementById("Setpoint").value = Setpoint;
            var pidstr1 = '#' + Kp.toString() + ',' + Ki.toString() + ',' + Kd.toString() + ',' + Setpoint.toString() + ',';
            Socket.send(pidstr1);
            t1 = performance.now();
            aktualisiert = 0;
        }
    }

    function sendPWM() {
        t0 = performance.now();
        var letzersend = t0 - t1;
        console.log("letzersend ist: " + letzersend);
        if (letzersend >= 50 || aktualisiert == 1) {
            var PWMwert = document.getElementById("PWMs").value;

            var pidstr7 = "A" + PWMwert.toString() + ',';
            console.log("PWM ist " + pidstr7);
            Socket.send(pidstr7);
            t1 = performance.now();
            aktualisiert = 0;
        }
    }
</script>

<body onload="javascript:start();">
</body>

</html>
)=====";