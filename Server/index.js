var chartTNow;
var chartTMonth;
var chartPNow;
var chartPMonth;
var chartPPMonth;
//var gateway = `ws://stocc.ddns.net:8081/`;
//var gateway = `wss://stodola-control-center.herokuapp.com:5000/`;
var gateway = location.origin.replace(/^http/, 'ws')
var websocket;

var PREDCTION_LEN = 16;
var AGREGATE_IN = 3;

var predChartData = [];

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

function setupCharts(){
    chartTNow = new Highcharts.Chart({
        chart:{ renderTo : 'chart-temperature-now' },
        title: { text: 'Temperatura teraz ESP32' },
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
            },
        credits: { enabled: false }
    });

    chartTMonth = new Highcharts.Chart({
        chart:{ renderTo : 'chart-temperature-month' },
        title: { text: 'Temperatura ostatni miesiac ESP32' },
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
        dateTimeLabelFormats: { second: '%b-%e %H:%M' }
        },
        yAxis: {
            title: { text: 'Temperatura &deg;C' }
            },
        credits: { enabled: false }
    });

    chartPNow = new Highcharts.Chart({
        chart:{ renderTo : 'chart-power-now' },
        title: { text: 'Moc pobierana teraz ESP32' },
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
            title: { text: 'Moc kWh' }
            },
        credits: { enabled: false }
    });

    chartPMonth = new Highcharts.Chart({
        chart:{ renderTo : 'chart-power-month' },
        title: { text: 'Moc pobierana ostatni miesiac ESP32' },
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
        dateTimeLabelFormats: { second: '%b-%e %H:%M' }
        },
        yAxis: {
            title: { text: 'Moc kWh' }
            },
        credits: { enabled: false }
    });

    chartPPMonth = new Highcharts.Chart({
        chart:{ renderTo : 'chart-power-pred-month' },
        title: { text: 'Moc pobierana predykcja ostatni miesiac ESP32' },
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
        dateTimeLabelFormats: { second: '%b-%e %H:%M' }
        },
        yAxis: {
            title: { text: 'Moc kWh' }
            },
        credits: { enabled: false }
    });
}

function setupDOM(){
    setupBackgroudVideo();
    setupCharts();   
          
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
    let data={
        type:"getPastValues"
    }
    websocket.send(JSON.stringify(data));   
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    try{
        const data = JSON.parse(event.data);
        if(data.msgType == "getValues"){
            document.getElementById('temp').innerHTML = data.temp.toFixed(2);
            document.getElementById('humi').innerHTML = data.humi;
            
            if(chartTNow.series[0].data.length > 2000) {
                chartTNow.series[0].addPoint([(new Date()).getTime(), data.temp], true, true, true);
            } else {
                chartTNow.series[0].addPoint([(new Date()).getTime(), data.temp], true, false, true);
            }

            if(chartPNow.series[0].data.length > 2000) {
                chartPNow.series[0].addPoint([(new Date()).getTime(), data.load], true, true, true);
            } else {
                chartPNow.series[0].addPoint([(new Date()).getTime(), data.load], true, false, true);
            }
        }
        else if(data.msgType == "getPastValues"){
            console.log("loadingData");
            chartTNow.series[0].setData(data.tempNow);
            chartPNow.series[0].setData(data.loadNow);
            console.log(data.tempMonth)
            chartTMonth.series[0].setData(data.tempMonth);
            chartPMonth.series[0].setData(data.loadMonth);
            predChartData = data.predNow;
            chartPPMonth.series[0].setData(data.predNow);
        }
        else if(data.msgType == "predUpdate"){
            let len = chartPPMonth.series[0].data.length;
            for(var i = 0; i < PREDCTION_LEN; i++){
                predChartData[len - PREDCTION_LEN + 1 + i] = [(new Date()).getTime() + 1000*60*60*AGREGATE_IN*i, data.value[i]];
            }
            chartPPMonth.series[0].setData(predChartData);
        }
        else if(data.msgType == "updateModelConsts"){
            PREDCTION_LEN = data.predLen;
            AGREGATE_IN = data.agreg;
        }
    }
    catch(e){
        console.log(e);
        console.log("Server message is not a JSON message");
    }
    
}

function getData() {
    let data={
        type:"getValues"
    }
    websocket.send(JSON.stringify(data));
}
