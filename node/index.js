console.log();
console.log("=================================================================");
console.log();
console.log(" fiOS ATTiny Auto Callibration");
console.log(" @version 1.0");
console.log(" @url http://triqadafi.com/");
console.log();
console.log("=================================================================");
console.log("Loading...");


const portName = process.argv[2];
const portBaudRate = 9600;
const SerialPort = require('serialport');
const parsers = SerialPort.parsers;


console.log(" + Serial Enabled");
//Use a `\r\n` as a line terminator
const callibrationParser = new parsers.Readline({
  delimiter: '\r\n'
});
const callibrationPort = new SerialPort(portName, {
  baudRate: portBaudRate
});
callibrationPort.on('open', callibrationPort_open);
callibrationPort.pipe(callibrationParser);
callibrationParser.on('data', callibrationPort_data);
callibrationPort.on('close', callibrationPort_close);
callibrationPort.on('error', callibrationPort_error);

function callibrationPort_open() {
  console.log('CALLIBRATION Port Open. Baud Rate: ' + portBaudRate);
}
var FI_OSCCALS = [];
function fi_remove(array, el){
  return array.filter(e => e !== el);
}
function callibrationPort_data(data) {
  //console.log(">>> " + data);
  var dataArray = data.split(";");
  if(dataArray[0] == "3913"){
    if(dataArray[1] == "ABCDEFGHIJKLMNOPQRSTUVWXYZ"){
      var echo = dataArray[0] + ";" + dataArray[1] + ";0;\r\n";
      callibrationPort.write(echo);
    }
    else if(dataArray[1] == "OSCCAL"){
      if(dataArray[3] == "0" && dataArray[4] == "0"){
        console.log("Detected! OSCCAL += " + dataArray[2] + ";");
        FI_OSCCALS.push(dataArray[2]);
      }else if(dataArray[4] == "1"){
        FI_OSCCALS = fi_remove(FI_OSCCALS, dataArray[2])
        console.log("OSCCAL += " + dataArray[2] + "; or OSCCAL = " + dataArray[3] + ";");
        var str = FI_OSCCALS.join(";") + ";";
        console.log(str);
      }
    }
    //console.log(echo);
  }

}
function callibrationPort_close() {
  console.log('CALLIBRATION Port Closed');
}
function callibrationPort_error(error) {
  console.log('CALLIBRATION Port ' + error);
}

var timekeeper = setInterval(()=>{}, 100000);
