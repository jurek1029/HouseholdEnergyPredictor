var chartT;
//var gateway = `ws://stocc.ddns.net:8081/`;
//var gateway = `wss://stodola-control-center.herokuapp.com:5000/`;
var gateway = location.origin.replace(/^http/, 'ws')
var websocket;

function setupBackgroudVideo(){
    var video = document.createElement('video');
    var t = Math.floor(Math.random() * 1663);
    video.src = "https://www.dropbox.com/s/9noro6gvmkvdohg/No-Audio.mp4?raw=1#t="+t;
    video.autoplay = true;
    video.controls = false;
    video.muted = true;
    video.loop = true;
                
    document.getElementById("vid").appendChild(video);
}

function setupChart(){
    chartT = new Highcharts.Chart({
        chart:{ renderTo : 'chart-temperature' },
        title: { text: 'Temperatura ESP32' },
        series: [{
        showInLegend: false,
        data: []
        }],
        plotOptions: {
        line: { animation: false,
            dataLabels: { enabled: true }
        },
        series: { color: '#059e8a' }
        },
        xAxis: { type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
        },
        yAxis: {
        title: { text: 'Temperatura &deg;C' }
        //title: { text: 'Temperature (Fahrenheit)' }
        },
        credits: { enabled: false }
    });
}

function setupDOM(){
    setupBackgroudVideo();
    setupChart();            
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
    console.log('Connection opened');
    setInterval(function() { getData(); }, 1500); //1500mSeconds update rate
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    if(event.data == "ON" || event.data == "OFF" || event.data == "wifi") {}
    else{
        const data = JSON.parse(event.data);
        if(data.msgType == "getValues"){
            document.getElementById('temp').innerHTML = data.temp.toFixed(2);
            document.getElementById('humi').innerHTML = data.humi;
            
            if(chartT.series[0].data.length > 2000) {
                chartT.series[0].addPoint([(new Date()).getTime(), data.temp], true, true, true);
            } else {
                chartT.series[0].addPoint([(new Date()).getTime(), data.temp], true, false, true);
            }
        }
    }
}

function getData() {
    websocket.send("getValues");
}
