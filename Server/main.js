var http = require('http');
var fs = require('fs');
var path = require('path');
const WebSocket = require('ws');
const PORT = process.env.PORT || 80;


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



const wsServer = new WebSocket.Server({
  //port: 3000
  server: server
});

var values = {
	msgType: "default",
	temp: 22,
	humi: 10,
    load: 0,
    predLoad: [0,0,0,0,0,0,0,0,0,0]
};

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
        else if(data.type == "load"){
            values.load = data.value;
        }
        else if(data.type == "temp"){
            values.temp = data.value;
        }
        else if(data.type == "humi"){
            values.humi = data.value;
        }
        else if(data.type == "pred"){
            values.predLoad = data.value;
        }
        else{
            console.log(`msg: ${msg}`);
        }
    }
    catch(e){
        console.log("Recived message that is not a valid JSON");
        console.log(`msg: ${msg}`);
    }
     
  });

  // When a socket closes, or disconnects, remove it from the array.
  socket.on('close', function() {
    sockets = sockets.filter(s => s !== socket);
  });
});
