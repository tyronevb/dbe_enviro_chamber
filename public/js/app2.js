var main = function(){
	//var counter = 5;
	var category = 0;
	//$('.form2').hide();	
	//$('#config-box').hide();
	$('#operation-box').hide();
	
    $('.btn-add-seg').click(function(){
		
		//var element_to_post = '<li><div class="row"><div class="col-sm-2"><p>Stage ' + counter + '<p></div><div class="col-sm-5"><textarea class="status-box" rows="1" placeholder="Target Temperature"></textarea></div><div class="col-sm-5"><textarea class="status-box" rows="1" placeholder="Time to target"></textarea></div></div></li>'
        //$(element_to_post).appendTo('.posts');
		//counter = counter + 1;
		$(this).hide();
		$('.form2').show();
    });
	
	$('.btn-stop').click(function(){
		$('#status-curr-reset').text("");
		
		/*$(this).addClass('disabled');
		$('#stopped-icon').removeClass('inactive-state')
		$('#running-icon').addClass('inactive-state')
		$('.btn-reset').removeClass('disabled');
		$('#running-icon').removeClass('fa-spin');
		$('#status-curr-stop').text("Halted");
		$('#status-curr-run').text("");*/			
	});
	
	$('.btn-reset').click(function(){
		$(this).addClass('disabled');
		$('#reset-icon').removeClass('inactive-state')
		$('#stopped-icon').addClass('inactive-state')		
		$('#operation-box').hide();
		$('#config-box').show();
		$('#status-curr-run').text("");
		$('#status-curr-stop').text("");
		$('#status-curr-reset').text("Configuration Ready");
		window.location.reload();		
	});
	
	
	$('.btn-confirm').click(function(){
		$('#reset-icon').addClass('inactive-state')
		$('#running-icon').removeClass('inactive-state')
		$('#status-curr-run').text("Ready");
		$('#status-curr-reset').text("Successfully Configured");
		$('.btn-start').removeClass('disabled');
		$('.btn-stop').removeClass('disabled');
	
	});
	$('.btn-start').click(function(){
		$(this).addClass('disabled');
		$('#status-curr-run').text("Running");
		$('#status-curr-reset').text("");
		$('#status-curr-stop').text("");
		$('#running-icon').addClass('fa-spin');
		$('.btn-stop').removeClass('disabled');
		$('#operation-box').show();
		$('#config-box').hide();
				
	});
	$('.btn-clear').click(function(){
		$('.btn-start').addClass('disabled');
		$('.btn-confirm').removeClass('disabled');
		$('#status-curr-run').text("");
		$('#status-curr-stop').text("");
		$('#status-curr-reset').text("Configuration ready");
		window.location.reload();		
	});
}
$(document).ready(main);