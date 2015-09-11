var stage_counter = 0;

var state = "idle";

function initSocketIO()
{
	iosocket = io.connect();
	iosocket.on('temp1', function(value) {
	$('#temp1').text(value.temp);
	
	if (state == "running")
	{
		// decrement time and update each time temp info is received
		total_time = total_time - 1;
		$('#time-total-min').text(total_time);
		
	}
	
	});	
	iosocket.on('temp2', function(value) {
	$('#temp2').text(value.temp);});	
	iosocket.on('temp3', function(value) {
	$('#temp3').text(value.temp);});
	iosocket.on('Completed', function(value){
		if (value.key == 'X')
		{
			alert("Test Cycle Completed!\n Click 'Stop' to reset controller");
			$(this).addClass('disabled');
			$('#stopped-icon').removeClass('inactive-state')
			$('#running-icon').addClass('inactive-state')
			$('.btn-reset').removeClass('disabled');
			$('#running-icon').removeClass('fa-spin');
			$('#status-curr-stop').text("Completed");
			$('#status-curr-run').text("");
			// reload page after completion
			//window.location.reload();
		}
		
		});
		
	iosocket.on("Stage", function (value){
		if(value.key == 'S')
		{
			stage_counter = stage_counter + 1;
			$('#current-stage').text(stage_counter);
			// update tag on front end to show current stage 
			
		}	
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
	for (var i = 1; i < 9; i++){
		//temperatureValues += ',';		
		var temp = document.getElementById("tempVal" + i.toString()).value;
		if (temp == ''){
			continue;
			//temperatureValues += 'NULL';	
		}
		else{
			temperatureValues += (temp + ',');			
		}
		$('#stage' + i.toString() + 'target').text(temp);
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
var total_time = 0;



function getTimeValuesFromBoxes(){
	timeValues += 'B,';		
	for (var i = 1; i < 9; i++){
		//timeValues += ',';		
		var temp = document.getElementById("timeVal" + i.toString()).value;
		if (temp == ''){
			continue;
			//timeValues += 'NULL';	
		}
		else{
			timeValues += (temp + ',');
			total_time = total_time + parseInt(temp);
			
		}
		
		
		$('#stage' + i.toString() + 'time').text(temp);
	}
	total_time++;
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
		state = "running";
	});
	$('.btn-stop').click(function() {
		state = "idle";
		$('#time-total-min').text("");
		$('#time-run-min').text("");
		var decision = confirm("Reset controller and cancel current run?");
			
			if (decision == true)
			{
				toggleVal2 = 1;					
				iosocket.emit('stop',toggleVal2);
				$(this).addClass('disabled');
				$('#stopped-icon').removeClass('inactive-state')
				$('#running-icon').addClass('inactive-state')
				$('.btn-reset').removeClass('disabled');
				$('#running-icon').removeClass('fa-spin');
				$('#status-curr-stop').text("Halted");
				$('#status-curr-run').text("");
				
				//window.location.reload();
				
			}
			else{
			}
		
	});
	$('.btn-reset').click(function() {
		toggleVal3 = 1;
		iosocket.emit('configure',toggleVal3);
		$('.btn-confirm').removeClass('disabled');
;	});
	$('.btn-confirm').click(function(){
		$(this).addClass('disabled');
		getTemperatureValuesFromBoxes();
		getTimeValuesFromBoxes();
		iosocket.emit('temperatureValues', temperatureValues);
		iosocket.emit('timeValues', timeValues);
	});
});