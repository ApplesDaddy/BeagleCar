//https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_client_applications

$(document).ready(function () {
    var ws = new WebSocket("ws://192.168.7.2:8080/ws");
    ws.onopen = function () {
        console.log("Socket has been opened!");
        ws.send("Hello, server!");
    };
    ws.onmessage = function (message) {
        console.log("New message: " + message.data);
        $("p").text("The value is " + message.data);
    };
});

