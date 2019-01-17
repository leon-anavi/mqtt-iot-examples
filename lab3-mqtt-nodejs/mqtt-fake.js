#! /usr/bin/env node

var mqtt = require('mqtt');
var client  = mqtt.connect('mqtt://localhost');

var topicPrefix = 'home/room'
var topicTemperature = topicPrefix + '/temperature';
var topicHumidity = topicPrefix + '/humidity';
var topicLight = topicPrefix + '/light';

var timeout = 5000;

client.on('connect', function () {
	client.subscribe(topicLight, function (err) {
		if (err) {
			console.log("Unable to subscribe.");
		}
	});
});

client.on('message', function (topic, message) {
	try {
		console.log("MQTT message received: "+message.toString());
	} catch (e) {
		console.log("Malformed data");
	}
});

function gentleExit() {
	client.end();
	process.exit();
}

// Handle ctrl+c and exit gently
process.on('SIGINT', gentleExit);

setInterval(function() {
	// Random temperature
	var temp = Math.floor(Math.random() * 11);
	console.log('Temperature: '+temp+'C');
	client.publish(topicTemperature, '{ "temperature": '+temp+' }');

	// Randrom humidity
	var humidity = Math.floor(Math.random() * 101);
	console.log('Humidity: '+humidity+'%');
	client.publish(topicHumidity, '{ "humidity": '+humidity+' }');
}, timeout);
