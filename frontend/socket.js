var io = require('socket.io').listen(3000);

io.sockets.on('connection', function (socket) {
    console.log("Someone just connected!");

    // Echo back messages from the client
    var onevent = socket.onevent;
    socket.onevent = function (packet) {
        var args = packet.data || [];
        onevent.call (this, packet);    // original call
        packet.data = ["*"].concat(args);
        onevent.call(this, packet);      // additional call to catch-all
    };

    socket.on("*",function(event,data) {
        socket.emit(event,data);
    });
});
