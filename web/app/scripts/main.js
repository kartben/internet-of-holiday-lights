client = new Paho.MQTT.Client("ws://iot.eclipse.org/ws", "id-" + new Date().getTime());
// client.onConnectionLost = onConnectionLost;
// client.onMessageArrived = onMessageArrived;

var connected = false;

function onConnect() {
	connected = true;
};

client.connect({
	onSuccess: onConnect
});

var color = null;

var cw = Raphael.colorwheel($(".colorwheel_large"), 500, 180).color("#00F");
cw.onchange(function(c) {
	color = c;
})

function sendColor() {
	if (connected) {
		if (color != null) {
			console.log('sending ' + color.toString())
			var message = new Paho.MQTT.Message(color.toString().replace('#', ''));
			message.destinationName = "benjamin-strip";
			client.send(message);
			color = null;
		}
	}
}

setInterval(sendColor, 100);