// https://www.npmjs.com/package/adm-zip
// https://nodejs.org/api/stream.html
// https://nodejs.org/api/net.html

var AdmZip = require("adm-zip");
const net = require("net");


const lenBuf = Buffer.allocUnsafe(4);


const client = net.createConnection({host: "192.168.1.99", port: 9876}, ()=>{
	console.log("Connected!");
	var zip = new AdmZip("./foo.zip");
	var zipEntries = zip.getEntries();
	zipEntries.forEach(zipEntry=>{
		console.log("Entry: " + zipEntry.entryName);
		lenBuf.writeUInt32LE(zipEntry.entryName.length, 0);
		client.write(lenBuf);
		client.write(zipEntry.entryName);
	});
	
	client.end();
});