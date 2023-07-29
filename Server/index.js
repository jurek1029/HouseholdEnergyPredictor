var chartTNow;
var chartTMonth;
var chartPNow;
var chartPMonth;
var chartPPMonth;
var chartPPredNow;
//var gateway = `ws://stocc.ddns.net:8081/`;
//var gateway = `wss://stodola-control-center.herokuapp.com:5000/`;
var gateway = location.origin.replace(/^http/, 'ws')
var websocket;

var PREDCTION_LEN = 16;
var AGREGATE_IN = 3;

var predChartData = [];
var predPowerChartData = [];

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

function setupTimeChart(data){
    var timeFormat = '%H:%M:%S';
    if(data.monthly) timeFormat = '%b-%e %H:%M';
    var chart = new Highcharts.Chart({
        chart:{ renderTo : data.renderTo },
        title: { text: data.title },
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
        dateTimeLabelFormats: { second: timeFormat }
        },
        yAxis: {
            title: { text: data.yAxis }
            },
        credits: { enabled: false }
    });
    return chart
}

function setupCharts(){
    chartTNow = setupTimeChart({
        title:'Temperatura teraz ESP32', 
        renderTo:'chart-temperature-now', 
        yAxis:'Temperatura &deg;C',
        monthly: false});

    chartTMonth = setupTimeChart({
        title:'Temperatura ostatni miesiac ESP32',
        renderTo:'chart-temperature-month',
        yAxis:'Temperatura &deg;C',
         monthly: true});

    chartPNow = setupTimeChart({
        title:'Moc pobierana teraz ESP32',
        renderTo:'chart-power-now',
        yAxis:'Moc kWh',
        monthly: false});

    chartPMonth = setupTimeChart({
        title:'Moc pobierana ostatni miesiac ESP32',
        renderTo:'chart-power-month',
        yAxis:'Moc kWh',
        monthly: true});

    chartPPMonth = setupTimeChart({
        title:'Moc pobierana predykcja ostatni miesiac ESP32',
        renderTo:'chart-power-pred-month',
        yAxis:'Moc kWh',
        monthly: true});

    chartPPredNow = setupTimeChart({
        title:'Predykcja produkcji energii',
        renderTo:'chart-prod-pred-now',
        yAxis:'Moc kWh',
        monthly: false});
        
}

function setupDOM(){
    //setupBackgroudVideo();
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
            
            //console.log(Object.keys(data));

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
            
            // Update Load prediction 
            let len = chartPPMonth.series[0].data.length;
            if(len > 2000){ //rotate if too many points so they will not by overwriten
                predChartData.shift();
            }
            let baseStart = Math.max(len - data.power.length + 1 ,0);
            for(var i = 0; i < data.predLoad.length; i++){ //update last values leaving one old and adding one new
                predChartData[baseStart + i] = data.predLoad[i];
            }
            chartPPMonth.series[0].setData(predChartData);

            // Update Power prediction
            len = chartPPredNow.series[0].data.length;
            if(len > 2000){ //rotate if too many points so they will not by overwriten
                predPowerChartData.shift();
            }
            baseStart = Math.max(len - data.power.length + 1 ,0);
            for(var i = 0; i < data.power.length; i++){ //update last values leaving one old and adding one new
                predPowerChartData[baseStart + i] = data.power[i];
            }
            chartPPredNow.series[0].setData(predPowerChartData);
        }
        else if(data.msgType == "getPastValues"){
            console.log("loadingData");
            chartTNow.series[0].setData(data.tempNow);
            chartPNow.series[0].setData(data.loadNow);
            chartTMonth.series[0].setData(data.tempMonth);
            chartPMonth.series[0].setData(data.loadMonth);
            chartPPMonth.series[0].setData(data.predNow);
            chartPPredNow.series[0].setData(data.powerNow);
        }
        // else if(data.msgType == "predUpdate"){
        //     let len = chartPPMonth.series[0].data.length;
        //     for(var i = 0; i < PREDCTION_LEN; i++){
        //         predChartData[len - PREDCTION_LEN + 1 + i] = [(new Date()).getTime() + 1000*60*60*AGREGATE_IN*i, data.value[i]];
        //     }
        //     chartPPMonth.series[0].setData(predChartData);
        // }
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
