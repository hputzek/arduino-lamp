let io = require('socket.io').listen(3000);

io.sockets.on('connection', function (socket) {
    console.log("Someone just connected!");

    // Echo back messages from the client
    let onevent = socket.onevent;
    socket.onevent = function (packet) {
        let args = packet.data || [];
        onevent.call (this, packet);    // original call
        packet.data = ["*"].concat(args);
        onevent.call(this, packet);      // additional call to catch-all
    };

    socket.on("*",function(event,data) {
        io.emit(event,data);
    });
});
