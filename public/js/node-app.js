function initSocketIO()
{
	iosocket = io.connect();
	iosocket.on('temp1', function(value) {
	$('#temp1').text(value.temp);});	
	iosocket.on('temp2', function(value) {
	$('#temp2').text(value.temp);});	
	iosocket.on('temp3', function(value) {
	$('#temp3').text(value.temp);
	});
}
			
function initButtons(){
	$('.btn-start').button();
	$('.btn-stop').button();
	$('.btn-reset').button();			
}

var socket = io();
initSocketIO();
initButtons();
var toggleVal1=0;
var toggleVal2=0;
var toggleVal3=0;
var timeValues = "";
var temperatureValues = "";

function getTemperatureValuesFromBoxes(){
	
	temperatureValues += 'A,';	
	for (var i = 1; i < 5; i++){
		//temperatureValues += ',';		
		var temp = document.getElementById("tempVal" + i.toString()).value;
		if (temp == ''){
			continue;
			//temperatureValues += 'NULL';	
		}
		else{
			temperatureValues += (temp + ',');			
		}
	}
	//temperatureValues += ',';
	temperatureValues += 'E';	
	
	
	/*temperatureValues += 'A';
	temperatureValues += ',';
	temperatureValues += document.getElementById("tempVal1").value;
	temperatureValues += ',';
	temperatureValues += document.getElementById("tempVal2").value;
	temperatureValues += ',';
	temperatureValues += document.getElementById("tempVal3").value;
	temperatureValues += ',';
	temperatureValues += document.getElementById("tempVal4").value;
	temperatureValues += ',';
	temperatureValues += 'E';*/
}

function getTimeValuesFromBoxes(){
	timeValues += 'B,';		
	for (var i = 1; i < 5; i++){
		//timeValues += ',';		
		var temp = document.getElementById("timeVal" + i.toString()).value;
		if (temp == ''){
			continue;
			//timeValues += 'NULL';	
		}
		else{
			timeValues += (temp + ',');			
		}
	}
	//timeValues += ',';
	timeValues += 'F';
	
	/*timeValues += 'B';	
	timeValues += ',';
	timeValues += document.getElementById("timeVal1").value;
	timeValues += ',';
	timeValues += document.getElementById("timeVal2").value;
	timeValues += ',';
	timeValues += document.getElementById("timeVal3").value;
	timeValues += ',';
	timeValues += document.getElementById("timeVal4").value;
	timeValues += ',';
	timeValues += 'F';*/
}
			
$(document).ready(function() {
	main();
	$('.btn-start').click(function() {
		toggleVal1 = 1;					
		iosocket.emit('run',toggleVal1);
	});
	$('.btn-stop').click(function() {
		toggleVal2 = 1;					
		iosocket.emit('stop',toggleVal2);
	});
	$('.btn-reset').click(function() {
		toggleVal3 = 1;
		iosocket.emit('configure',toggleVal3);
	});
	$('.btn-confirm').click(function(){
		$(this).addClass('disabled');
		getTemperatureValuesFromBoxes();
		getTimeValuesFromBoxes();
		iosocket.emit('temperatureValues', temperatureValues);
		iosocket.emit('timeValues', timeValues);
	});
});