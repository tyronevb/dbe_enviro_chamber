var app = require('express')();
var http = require('http').createServer(app);
var socketServer = require('socket.io').listen(http);
var fs = require('fs');
//var csv_stream = fs.createWriteStream("tempLog.csv");
var time_handler = require('moment');

// needs to be global
var fileName = 'temp'
var csv_stream = fs.createWriteStream(fileName)

var run_number = 0;


//console.log(time_handler().format('DD/MM/YYY');
SerialPort = require("serialport").SerialPort;

var serialPort;
var portName = '/dev/ttyACM0'; //change this to your Arduino port
var sendData = "";
var debug = false; 

// flag for file writing
var file_write = false;

app.get('/', function(req, res){
	res.sendFile(__dirname + '/public/interface.html')
});

socketServer.on('connection', function(socket){
  console.log('a user connected');
	socket.on('run', function(data){
	  serialPort.write('Z' + data + 'A');
	  console.log('write to ard');
	  console.log('Z' + data + 'A');
	  fileName = (__dirname + '/logs/Run:' + time_handler().format('DD-MMM-YY_HH:mm:ss') + '_' + run_number + '.csv')
 	  csv_stream = fs.createWriteStream(fileName)
          run_number += 1;
          file_write = true;
	});
	socket.on('stop', function(data){
	  serialPort.write('Y' + data + 'B');
	  console.log('write to ard');
	  console.log('Y' + data + 'B');
      file_write = false;
	});
	socket.on('configure', function(data){
	  serialPort.write('X' + data + 'C');
	  console.log('write to ard');
	  console.log('X' + data + 'C');
	});
	socket.on('temperatureValues', function(data){
		serialPort.write(data);
		console.log('write data to ard');
		console.log(data);		
	});
	socket.on('timeValues', function(data){
		serialPort.write(data);
		console.log('write data to ard');
		console.log(data);		
	});
});

serialListener(debug);

function SocketIO_serialemit(sendData){      
	//console.log(sendData.length);     

	if(sendData.length >= 14)
	{	
		var s_data = sendData.split(",");
		socketServer.emit('temp1',{'temp': s_data[0]});
		socketServer.emit('temp2',{'temp': s_data[1]});
		socketServer.emit('temp3',{'temp': s_data[2]});
        
        // only write to the file if a control program is running
        if (file_write == true)
        {
		    csv_stream.write(time_handler().format('HH:mm:ss') + ',');
		    csv_stream.write(s_data[0] + ',' + s_data[1] + ',' + s_data[2]);
		    csv_stream.write('\n');
		    //csv_stream.end();	
        }
	}
	
	else{
		console.log("Error");
		//var s_data = sendData.split(",");
		//console.log(s_data[0]);
		//console.log(s_data[1]);
		//console.log(s_data[2]); 	
	}
}

// Listen to serial port
function serialListener(debug)
{
    var receivedData = "";
    serialPort = new SerialPort(portName, {
        baudrate: 9600,
        // defaults for Arduino serial communication
         dataBits: 8,
         parity: 'none',
         stopBits: 1,
         flowControl: false
    });
 
    serialPort.on("open", function () {
      console.log('open serial communication');
            // Listens to incoming data
			serialPort.on('data', function(data) {
				
			if (data.toString() == "X")
			{
				console.log("Control Done");
				socketServer.emit('Completed',{'key': data.toString()});

                // run is completed here, so stop writing to the file
                file_write = false;
			}
			
			if (data.toString()[0] == "S")
			{
				console.log("Stage Done");
				socketServer.emit('Stage',{'key': data.toString()[0]});
			}
			
			receivedData += data.toString();
			
			  //console.log(receivedData);
			  if (receivedData .indexOf('E') >= 0 && receivedData .indexOf('T') >= 0) {
			   sendData = receivedData .substring(receivedData .indexOf('T') + 1, receivedData .indexOf('E'));        
			receivedData = '';
			SocketIO_serialemit(sendData);
			 }
			 
			 
		});  
    });
}

http.listen(3000, function(){
	console.log('listening on *:3000')	
});

app.use(require('express').static(__dirname + '/public')); //sever static files (css, js, fonts, etc.)
