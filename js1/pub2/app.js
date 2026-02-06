(function() {
	let address = "localhost";
	address = window.location.toString().replace("http://", "").replace(":3001", "");
	address = address.substring(0, address.length-1);
	alert(address)
	const ws = new WebSocket("ws://" + address+ ":5000");
	const tb = document.getElementById("tb");
	const core = document.getElementById("core");
	function resize() {
		const w = window.innerWidth, h = window.innerHeight;
		let g = core.style, f = tb.style;
		let ar = 16 / 9;
		f.width = `${h * (ar)}px`;
		//f.height = `${h*(1)}px`;
		g.width = `${w * (1)}px`;
		g.height = `${h * (1)}px`;
	}
	addEventListener("resize", resize);

	ws.onopen = () => {
		console.log("OPEN");
		ws.send("login");
	}
	ws.onmessage = (e) => {
		let a = e.data;
		refresh(JSON.parse(a));
	};

	let details = ["date", "time", "name", "scantype"];

	resize();
	function refresh(data) {
		tb.innerHTML = "";
		let d = ["_", ...Object.keys(data)];
		for (let _g = 0; _g < Math.max(60, d.length); _g++) {
			let ref = {};
			for (let o of details) {
				ref[o] = null;
			}
			let g = d[_g];
			if (_g < d.length) {
				ref = data[g];
			}
			let tr = document.createElement("tr");
			for (let h of details) {
				let td = document.createElement(g == "_" ? "th" : "td");
				td.style.border = "3px solid #000";
				td.style.height = "1em";
				let j = g || " ";
				if (g == "_") {
					j = h;
				} else {
					if (h !== "key" && ref[h] !== null) {
						switch (h) {
							case "time": {
								let time = new Date(ref[h]);
								let hour = time.getHours();
								let min = time.getMinutes();
								let zero = min < 10 ? "0" : "";
								let hz = hour < 10 ? "0" : "";
								j = ref?.[h] ? `${hz}${hour}:${zero}${min}` : "N/A";
								break;
							}
							case "last_time_scanned": {
								let time = new Date(ref[h]);
								let hour = time.getHours();
								let min = time.getMinutes();
								let zero = min < 10 ? "0" : "";
								let hz = hour < 10 ? "0" : "";
								j = ref?.[h] ? `${hz}${hour}:${zero}${min}` : "N/A";
								break;
							}
							default: {
								j = ref[h] || "N/A";
							}
						}
					}
				}

				td.innerHTML = j; //h == "key"? g: (ref[h] || "N/A");

				tr.append(td);
			}
			tb.append(tr);
		}
	}
})();