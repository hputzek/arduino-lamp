var io = require('socket.io').listen(3000);

io.sockets.on('connection', function (socket) {
    console.log("Someone just connected!");

    // Echo back messages from the client
    socket.on('message', function (message) {
        console.log("Got message: " + message);
        socket.emit('message', message);
    });
});
