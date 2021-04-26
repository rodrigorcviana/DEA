var app = require("express")();
var http = require("http").Server(app);
var io = require("socket.io")(http);

const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');

const port = new SerialPort('/dev/ttyACM0', { baudRate: 115200 });

const parser = port.pipe(new Readline({ delimiter: '|'}));
// Read the port data
port.on("open", () => {
  console.log('serial port open');
});


var output = {};

var canalSweepFlag, FFFlag, FCFlag;

parser.on("data", function(data){
  if(data.length == 3 && (!FCFlag && !FFFlag)){
    canalSweepFlag = false;
    output.csFlag = canalSweepFlag;
    FFFlag = false;
    FCFlag = false;
  }
  if(data.length == 2){
    if(data == "FS"){
      canalSweepFlag = true;
      output.csFlag = canalSweepFlag;
      FFFlag = false;
      FCFlag = false;
    }
    if(data == "FF"){
      FFFlag = true;
      FCFlag = false
    }

    if(data == "FC"){
      FCFlag = true;
      FFFlag = false;
    }
  }
  if(data.length < 27){
    if(FCFlag){
      output.canal1 = parseFloat(data);
    }
    if(FFFlag){
      output.frequencySweep = parseFloat(data);
    }
  }
  if(data.length == 27){
    var trataDados = data.split(",");
    
    if(canalSweepFlag){
      output.freal = parseFloat(trataDados[0]);
      output.fim = parseFloat(trataDados[1]);
    }
    else{
      output.creal = parseFloat(trataDados[0]);
      output.cim = parseFloat(trataDados[1]);
    }

    //for(var i = 0; i < 1000; i++);

    //console.log("leitura: "+ output.canal1 + "  " + output.creal + "  " + output.cim);    
    //console.log("leitura: "+ output.frequencySweep + "  " + output.freal + "  " + output.fim);
  }
});


io.on("connection", function(socket){
    socket.on('leitura?', data => {
      socket.emit('leitura', output);
    });

    socket.on('abrir-comando', data => {
      console.log(data);
      port.write('a');
      parser.on("data", datafb => {
        setTimeout(() => {
            if(datafb == "ok"){
                socket.on('comando-front', datacmd => {
                    console.log(datacmd);
                    port.write(datacmd);
                    parser.on("data", datacmdfb => {
                        socket.emit('abrir-comandofb', datacmdfb);
                    })
                });
            }

            else socket.emit('abrir-comandofb', "nok");
                
        }, 150)
      })
    })
});

http.listen(3000, function(){
  console.log("O servido esta funcionando na porta 3000");
});