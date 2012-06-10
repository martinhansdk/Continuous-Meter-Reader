var io = require('socket.io').listen(8000); // server listens for socket.io communication at port 8000
io.set('log level', 1); // disables debugging. this is optional. you may remove it if desired.


var SerialPort  = require('serialport').SerialPort;
var portName = '/dev/ttyUSB1';

var sp = new SerialPort(portName); // instantiate the serial port.
var counterValue = 'waiting'; // this stores the clean data
var readData = '';  // this stores the buffer
sp.on('data', function (data) { // call back when data is received
    readData += data.toString(); // append data to buffer
    // if the characters '<' and '>' are found on the buffer then isolate what's in the middle
    // as clean data. Then clear the buffer. 
    var openTag = readData.indexOf('<');
    var closeTag = readData.indexOf('>');
    if (openTag >= 0 && closeTag >= 0) {
        counterValue = parseInt(readData.substring(openTag + 1, closeTag)) * 0.001;
        readData = '';
        io.sockets.emit('counter', counterValue);
    }
});

io.sockets.on('connection', function (socket) {
    console.log('connected');
    socket.emit('counter', counterValue);
    // If socket.io receives message from the client browser then 
    // this call back will be executed.
    socket.on('message', function (msg) {
        console.log(msg);
    });
    // If a web browser disconnects from Socket.IO then this callback is called.
    socket.on('disconnect', function () {
        console.log('disconnected');
    });
});
