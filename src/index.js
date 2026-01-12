const ws = require("ws");
const http = require("http");
const fs = require("fs");
//sample

const web = http.createServer((req, res) => {
    if (req.url == "/") {
        let s = fs.readFileSync("index.html"); 
        if (s.toString) {
            res.writeHead(200);
            res.end(s);
        }
    }
});



web.listen(3305, () => {
    console.log("server listening on 3305");
});
const server = new ws.Server({port:3306});