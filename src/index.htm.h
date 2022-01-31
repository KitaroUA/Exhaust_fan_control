const char index_htm[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>Exhaust FAN Monitor</title>
  <link rel="stylesheet" type="text/css" href="style.css">
  <script type="text/javascript" src="script.js"></script>

  <script src="https://code.jquery.com/jquery-3.2.1.min.js"></script>
  <script>    window.jQuery || document.write('<script src="../js/jquery-3.2.1.min.js"><\/script>')</script>
</head>


<script type="text/javascript">
  function get_data(){	
  $.get("index_get_data" , {sensor_data:"get_data"}, function( data ) {
  //	$("#working_time").html(data);
  //	$("#current_time").load(window.location.href + " #temperature");
      var myVars;
      var line=data.split(';');
//      console.log(data);
      myVars={
          'temperature':          line[0],
          'slider_value':         line[1],
          'weighted_temperature': line[2],
          'autoPWM':              line[3]
      }
      $("#temperature").html("Температура : "+myVars.temperature+"°C");
      $("#pwmSlider").prop("value", myVars.slider_value);
      
      
      if (myVars.autoPWM == "1"){
          $("#pwmSlider").prop("disabled", true);
          $("#textSliderValue").html("FAN: " + myVars.weighted_temperature+"&percnt;");
          $("#slider1" ).prop( "checked", true );
          $("#checker_1_text").html("Auto ");

        }
        else {
          $("#pwmSlider").prop("disabled", false);
          $("#textSliderValue").html("FAN: " + myVars.slider_value+"&percnt;");
          $("#slider1" ).prop( "checked", false );
          $("#checker_1_text").html("Manual");

        }

 
        $("#onoffswitch1").load(window.location.href + "#onoffswitch1");

    
  console.log(myVars);
  });
  }
   </script>


<script type="text/javascript">
  setInterval("get_data()", 15000); 
   </script>


<script type="text/javascript">
  function load_script() {
   get_data();
  
  };

  </script>
     
   <body onload="load_script()">
  <h2>Exhaust FAN Monitor</h2>
  <div id="textSliderValue"></div>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="%SLIDERMAXVALUE%" step="%SLIDERSTEP%" class="slider"></p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <div id="temperature"></div>
  </p>
  
  <tr>
    <td align="right">
      <div class="onoffswitch1">
      <input type="checkbox" class="onoffswitch1-checkbox" id="slider1" name="slider1" onchange="updateSwitch(this)" > 
        <label class="onoffswitch1-label" for="slider1"> 
              <span class="onoffswitch1-inner"></span>
              <span class="onoffswitch1-switch"></span>
      </label>
      </div>
      </td>


    
    
    <td align="left"><font size="5" class="switch_text"><div id="checker_1_text"></div></font></td>
  </tr>
  
  
  <p style="text-align: center"><button style="height:11px;width:25px;vertical-align:bottom;" onclick="Reset_ESP()"></button></p>







</body>
</html>
)rawliteral";