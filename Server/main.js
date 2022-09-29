var http = require('http');
var fs = require('fs');
var path = require('path');
const WebSocket = require('ws');
const { PassThrough } = require('stream');
const PORT = process.env.PORT || 80;
const PREDCTION_LEN = 16;
const MAX_NOW_VALUES = 2000;
const MAX_MONTH_VALUES = 24/3 * 31 * 2;
const AGREGATE_IN = 3;

var lastAgregation = {
    load: 0,
    temp: 0,
};

var lastAgregationValue = {
    load: 0,
    temp: 0,
};

var lastAgregationValueCount = {
    load: 0,
    temp: 0,
};


var values = {
	msgType: "default",
	temp: 22,
	humi: 10,
    load: 0,
};

var pastValues;

const storeData = (data, path) => {
    try {
        fs.writeFileSync(path, JSON.stringify(data))
    } catch (err) {
        console.error(err)
    }
}

const loadData = (path) => {
    try {
        return fs.readFileSync(path, 'utf8')
    } catch (err) {
        console.error(err)
        return false
    }
}

function setupData(){
    pastValues = JSON.parse(loadData("monthly.data"));
}

function saveData(){
    storeData(pastValues,"monthly.data");
}

var server = http.createServer(function (request, response) {
    console.log('request ', request.url);
	
	if(request.method == 'POST'){
		var body = '';
        request.on('data', function (data) {
            body += data;
        });
        request.on('end', function () {
            try {
				console.log(body);
				var post = JSON.parse(body);
				console.log(post);
                return;
            }catch (err){
				console.log(err);
                response.writeHead(500, {"Content-Type": "text/plain"});
                response.write("Bad Post Data.  Is your data a proper JSON?\n");
                response.end();
                return;
            }
        });
	}
	
	
	var filePath = '.' + request.url;
    if (filePath == './') {
        filePath = './index.html';
    }

    var extname = String(path.extname(filePath)).toLowerCase();
    var mimeTypes = {
        '.html': 'text/html',
        '.js': 'text/javascript',
        '.css': 'text/css',
        '.json': 'application/json',
        '.png': 'image/png',
        '.jpg': 'image/jpg',
        '.gif': 'image/gif',
        '.svg': 'image/svg+xml',
        '.wav': 'audio/wav',
        '.mp4': 'video/mp4',
        '.woff': 'application/font-woff',
        '.ttf': 'application/font-ttf',
        '.eot': 'application/vnd.ms-fontobject',
        '.otf': 'application/font-otf',
        '.wasm': 'application/wasm'
    };

    var contentType = mimeTypes[extname] || 'application/octet-stream';

    fs.readFile(filePath, function(error, content) {
        if (error) {
            if(error.code == 'ENOENT') {
                fs.readFile('./404.html', function(error, content) {
                    response.writeHead(404, { 'Content-Type': 'text/html' });
                    response.end(content, 'utf-8');
                });
            }
            else {
                response.writeHead(500);
                response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
            }
        }
        else {
            response.writeHead(200, { 'Content-Type': contentType });
            response.end(content, 'utf-8');
        }
    });

}).listen(PORT);

//server.listen(PORT);
console.log('Server running at http://127.0.0.1:80/');
setupData();


const wsServer = new WebSocket.Server({
  //port: 3000
  server: server
});

function updateNow(key,value){
    if(pastValues[key+"Now"].length > MAX_NOW_VALUES){
        pastValues[key+"Now"].shift();
    }
    pastValues[key+"Now"].push([(new Date()).getTime(),value]);

    lastAgregationValue[key] += value;
    lastAgregationValueCount[key] += 1;
    if((new Date()).getTime() > lastAgregation[key] + 1000*60*60*AGREGATE_IN){
        if(pastValues[key+"Month"].length > MAX_MONTH_VALUES){
            pastValues[key+"Month"].shift();
        }
        pastValues[key+"Month"].push([(new Date()).getTime(), lastAgregationValue[key] / lastAgregationValueCount[key]]);
        lastAgregationValue[key] = 0 ;
        lastAgregationValueCount[key] = 0;
        lastAgregation[key] = (new Date()).getTime();
    }
    saveData();
}

function updatePredNow(values){
    if(pastValues.predNow.length > MAX_NOW_VALUES + PREDCTION_LEN){
        pastValues.predNow.shift();
    }
    //overrite last 15 predictions and add new;
    for(let i = 0; i < PREDCTION_LEN; i++){
        pastValues.predNow[pastValues.predNow.length - PREDCTION_LEN + 1 + i] = [(new Date()).getTime() + 1000*60*60*AGREGATE_IN*i, values[i]];
    }
    saveData();
}

let sockets = [];
wsServer.on('connection', function(socket) {
  sockets.push(socket);

    socket.on('message', function(msg) {
        
        try{
            var data = JSON.parse(msg);
            if(data.type == "getValues"){
                values.msgType = "getValues";
                socket.send(JSON.stringify(values));
            }
            else if(data.type == "getPastValues"){
                console.log("sending past data");
                socket.send(JSON.stringify(pastValues));
            }
            else if(data.type == "load"){
                values.load = data.value;
                updateNow("load",data.value);
            }
            else if(data.type == "temp"){
                values.temp = data.value;
                updateNow("temp",data.value);
            }
            else if(data.type == "humi"){
                values.humi = data.value;
            }
            else if(data.type == "pred"){
                values.predLoad = data.value;
                updatePredNow(data.value);
                d = {
                    msgType: "predUpdate",
                    value: data.value
                };
                sockets.forEach(function(s){
                    if (s != socket){
                        s.send(JSON.stringify(d));
                    }
                });
            }
            else{
                console.log(`msg: ${msg}`);
            }
        }
        catch(e){
            console.log(e);
            console.log("Recived message that is not a valid JSON");
            console.log(`msg: ${msg}`);
        }
        
    });

  // When a socket closes, or disconnects, remove it from the array.
  socket.on('close', function() {
    sockets = sockets.filter(s => s !== socket);
  });
});
