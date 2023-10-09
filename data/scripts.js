function selectTimeZone(element) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "timezonechange?output="+element.id+"&state=1", true);
    xhttp.send();
  }


function changeActivity(element){
var xhttp = new XMLHttpRequest();
if(element.id == "next"){
  xhttp.open("GET", "changeactivity?output=1", true);
  xhttp.send();
}else if (element.id == "previous"){
  xhttp.open("GET", "changeactivity?output=0", true);
  xhttp.send();
}

}






