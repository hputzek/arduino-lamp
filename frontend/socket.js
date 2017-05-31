// testServer.js

const Server = require('vue-remote/server');

function messageHandler(message) {
    // message = {
    //    identifier: "trigger",
    //    arguments: [...]
    // }


    return {
        identifier: "trigger",
        data: "Handled Message"
    };
}

let socketServer = Server(messageHandler);
