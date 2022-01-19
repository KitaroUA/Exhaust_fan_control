const char script_js[] PROGMEM = R"rawliteral(


function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}

function updateSliderPWM(element) {
  var sliderValue = document.getElementById("pwmSlider").value;
  document.getElementById("textSliderValue").innerHTML = "FAN: " + sliderValue+"&percnt;";
  document.getElementById("pwmSlider").value = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue, true);
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temp_sensor").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temp_sensor", true);
  xhttp.send();
}, 10000) ;


function Reset_ESP ()
{
//  var xhr = new XMLHttpRequest();
//  xhr.open("GET", "/resetesp", true);
//  xhr.send();
$.post("resetesp");
}

)rawliteral";