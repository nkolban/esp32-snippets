// Node.js application for comparing two ESP-IDF configuration files (sdkconfig)
const fs       = require("fs");            // Require the file system processing library
const readline = require("readline");      // Require the readline processing library

// buildMap
// Read the sdkconfig file specified by fileName and produce a map of the name/value pairs contained
// within.  A Promise is returned that is fulfilled when the file has been read.
function buildMap(fileName) {
	const promise = new Promise(function(resolve, reject) {
		var readStream = fs.createReadStream(fileName);
		readStream.on("error", (err) => {
			reject(err);
		});
		const map = {};

		const lineReader = readline.createInterface({
			input:     readStream,
			crlfDelay: Infinity
		});
		
		// Called when a new line has been read from the file.
		lineReader.on("line", (line) => {
			line = line.trim();              // Trim whitespace from the line.
			
			if (line.length == 0) {          // Ignore empty lines
				return;
			}
			if (line.startsWith("#")) {      // Ignore comment lines
				return;
			}

			const parts = line.split("=");   // Split the line into parts separated by the '=' character.
			if (map.hasOwnProperty(parts[0])) {
				console.log(`Odd ... we found ${parts[0]} twice.`);
			}
			map[parts[0]] = parts[1];        // Populate the map element.
		}); // on(line)

		// Called when all the lines from the file have been consumed.
		lineReader.on("close", () => {
			resolve(map);
		}); // on(close)

	});
	return promise;
} // buildMap


const args = process.argv;
if (args.length != 4) {
	console.log("Usage: node sdkconfig_compare file1 file2");
	process.exit();
}
const file1 = args[2];
const file2 = args[3];
buildMap(file1).then((result)=> {
	buildMap(file2).then((result2) => {

		// Three passes
		// In A and not B
		// in B and not A
		// value different in A and B
		for (const prop in result) {
			if (result.hasOwnProperty(prop)) {
				if (!result2.hasOwnProperty(prop)) {
					console.log(`${prop} in ${file1} but not in ${file2}`);
				}
			}
		}
		
		for (const prop in result2) {
			if (result2.hasOwnProperty(prop)) {
				if (!result.hasOwnProperty(prop)) {
					console.log(`${prop} in ${file2} but not in ${file1}`);
				}
			}
		}
		
		for (const prop in result) {
			if (result.hasOwnProperty(prop)) {
				if (result2.hasOwnProperty(prop)) {
					if (result[prop] != result2[prop]) {
						console.log(`${prop} values different "${result[prop]}" vs "${result2[prop]}"`);
					}
				}
			}
		}
	}).catch((err) => {
		console.log(err);
		process.exit();
	});
}).catch((err) => {
	console.log(err);
	process.exit();
});