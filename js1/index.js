const ws = require("ws");
const fs = require("fs");
const express = require("express");
const Serial = require("serialport");

const app = express();

const PORT = 3000;
const WS_PORT = 5000;

const arduino = new Serial.SerialPort({
	path: "/dev/ttyUSB0",
	baudRate: 9600
})

const wss = new ws.Server({ port: WS_PORT });

app.use(express.static("./pub"));

app.listen(PORT, () => {
	console.log("Frontend Server on port " + PORT + " with server port 5000");
});


const app2 = express();

app2.use(express.static("./pub2"));

app2.listen(PORT + 1, () => {
	console.log("Frontend Server on port " + (PORT + 1) + " with server port 5000");
});



let masterlist = JSON.parse(fs.readFileSync("users.json", "utf8"));
let login = JSON.parse(fs.readFileSync("login.json", "utf8"));
console.log(masterlist);

const last = {
	id: "",
	date: "",
	time: 0

}

setTimeout(() => {
	arduino.write("fail");
}, 1000);

let isDBRefreshable = true;
function refreshDatabase() {
	if (isDBRefreshable) masterlist = JSON.parse(fs.readFileSync("users.json", "utf8"));
	isDBRefreshable = true;
}

setInterval(() => {
	refreshDatabase();
}, 6000);

arduino.on("data", (e) => {
	isDBRefreshable = false;
	let d = e.toString().trim().replace("\x00", "");

	//let d = JSON.parse(l).content;
	console.log(d);
	//arduino.write("success");
	if (d in masterlist) {
		let user = masterlist[d];
		user.date_scanned = (new Date().toJSON()).split("T")[0];//.getUTCDate();
		user.time_scanned = Date.now();
		if (!("scans" in user)) {
			user.scans = 0;
		}
		let canAccess = false;
		let delta = Date.now() - (user?.last_time_scanned || 0);
		if (user?.time_scanned !== user.last_time_scanned && (delta / 1000) > 10) {
			user.last_time_scanned = user?.time_scanned;
			canAccess = true;
		}
		if (user?.date_scanned !== user.last_date_scanned) {
			user.last_date_scanned = user?.date_scanned;
			canAccess = true;
			user.scans = 0;
		}
		let isUnavailableScan = false;
		if (user?.scans >= 2) {
			//user.last_date_scanned = user?.date_scanned;
			canAccess = false;
			isUnavailableScan = true;
		}

		if (canAccess) {
			arduino.write("success");
			console.log(`Hello, ${user.name}!`);
			user.scans++;
			login.push({
				time: user.time_scanned,
				date: user.date_scanned,
				name: user.name,
				scantype: ["NIL", "IN", "OUT"][user.scans]
			});

		} else {
			arduino.write(isUnavailableScan ? "maxout" : "cooldown");

		}
		//arduino.write("success");
		fs.writeFile("users.json", JSON.stringify(masterlist, "\n", " "), () => { });



		fs.writeFile("login.json", JSON.stringify(login, "\n", " "), () => { });

	} else {
		arduino.write("fail");
		console.log("not found");
	}

	broadcast("database", cl => {
		cl.send(JSON.stringify(masterlist));
		console.log(cl.uid, d)
	});

	broadcast("login", cl => {
		cl.send(JSON.stringify(login));
		console.log(cl.uid, d)
	});
});

let clients = {};
wss.on("connection", (client) => {
	client.type = "";
	client.uid = ~~(2147483647 * Math.random());
	clients[client.uid] = client;
	console.log(clients)

	client.on("message", message => {
		if (message == "database") {
			client.type = "database";
			client.send(JSON.stringify(masterlist));
		}
		if (message == "login") {
			client.type = "login";
			client.send(JSON.stringify(login));
		}
	});

	client.on("close", message => {
		delete clients[client.id];
	});
});

function broadcast(type, func) {
	console.log("BROADCAST");
	for (let g in clients) {
		if (clients[g].type !== type) continue;
		func(clients[g]);
	}
}