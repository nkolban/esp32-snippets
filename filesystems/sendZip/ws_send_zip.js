// https://www.npmjs.com/package/adm-zip
// https://nodejs.org/api/stream.html
// https://nodejs.org/api/net.html
//API docs ...https://github.com/websockets/ws/blob/master/doc/ws.md

const WebSocket = require("ws");
var AdmZip = require("adm-zip");

function *processZip(zipFileName) {
	var zip = new AdmZip("./foo.zip");
	var zipEntries = zip.getEntries();
	for (var i=0; i<zipEntries.length; i++) {
		var zipEntry = zipEntries[i];
		//console.log("Entry: " + zipEntry.entryName);
		//if (!zipEntry.isDirectory) {
			yield {name: zipEntry.entryName, data: zip.readFile(zipEntry)};
		//}
	}
}

/**
 * Send a file entry.
 * @param entry The file to write.  An object that contains:
 * ```
 * {
 *    "name": <string>, // The file name to send
 *    "data": <data>,   // The content of the file
 * }
 * ```
 * @returns N/A
 */
function sendZipEntry(entry) {
	const ws = new WebSocket("ws://192.168.1.99:9080/upload");
	ws.on("open", ()=>{
		console.log("Sending file: " + entry.name);
		var file = {
			"name": entry.name
			//"length": 100
		}
		ws.send(JSON.stringify(file));
		ws.send(entry.data);
		ws.close();
	});
	
	ws.on("error", (error)=>{
		console.log("Error: %O", error);
	});
	
	ws.on("close", (code)=>{
		console.log("WS closed");
		var nextEntry = it.next();
		if (!nextEntry.done) {
			sendZipEntry(nextEntry.value);
		}
	});
} // sendZipEntry


var it = processZip("./foo.zip");
var nextZip = it.next();
if (!nextZip.done) {
	sendZipEntry(nextZip.value);
}
