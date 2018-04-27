/**
 * Requires:
 * * hexy
 * * serialport - https://www.npmjs.com/package/serialport
 * 
 * To debug, run:
 * node --inspect-brk espjs.js
 * 
 * In chrome run, 
 * 
 * chrome://inspect
 * 
 */

 /**
  * High level commands:
  * doSync - SYNC
  * MemCommands
  * - MEM_BEGIN
  * - MEM_END
  * FlashCommands
  *  - FLASH_BEGIN
  *  - FLASH_END
  * writeReg - WRITE_REG
  * readReg - READ_REG
  * enterDownloadMode
  * spiSetParams - SPI_SET_PARAMS
  * flashFile - Flash a specific file to a specific address.
  * spiFlashMD5 - Calculate the hash of a region of flash.
  */

 /**
  * We want to provide processing to both transmitted requests and received responses.  A request is a message that is then encapsulated
  * in a SLIP packet and transmitted via UART.  A response is a message received via UART that is wrapped in a SLIP packet.
  * 
  * We will logically define a DataPacket as the raw packet unit that is either transmitted or received.  This encapsulates a
  * Buffer that contains the data.  
  */
const hexy       = require("hexy");                                           // Dumping hex data utility.
const zlib       = require('zlib');                                           // Compression/decompression.
const fs         = require("fs");                                             // File system access.
const assert     = require("assert");                                         // Internal assertion.
const argv       = require("minimist")(process.argv.slice(2));                // command line processing.
const SerialPort = require("serialport");                                     // Serial / UART access.
const md5        = require("md5");                                            // Hash checksum.

const PORT       = "/dev/ttyUSB1";                                            // The serial port to work with.

// These are the commands exposed by the Flasher stub.
const COMMAND_FLASH_BEGIN      = 0x02;
const COMMAND_FLASH_DATA       = 0x03;
const COMMAND_FLASH_END        = 0x04;
const COMMAND_MEM_BEGIN        = 0x05;
const COMMAND_MEM_END          = 0x06;
const COMMAND_MEM_DATA         = 0x07;
const COMMAND_SYNC             = 0x08;
const COMMAND_WRITE_REG        = 0x09;
const COMMAND_READ_REG         = 0x0a;
const COMMAND_SPI_SET_PARAMS   = 0x0b;
const COMMAND_SPI_ATTACH       = 0x0d;
const COMMAND_CHANGE_BAUDRATE  = 0x0f;
const COMMAND_FLASH_DEFL_BEGIN = 0x10;
const COMMAND_FLASH_DEFL_DATA  = 0x11;
const COMMAND_FLASH_DEFL_END   = 0x12;
const COMMAND_SPI_FLASH_MD5    = 0x13;
const COMMAND_ERASE_FLASH      = 0xd0;
const COMMAND_ERASE_REGION     = 0xd1;
const COMMAND_READ_FLASH       = 0xd2;
const COMMAND_RUN_USER_CODE    = 0xd3;

const EFUSE_REG_BASE           = 0x6001a000;

var esp32r0Delay = false;
var response     = null;
var serialData   = SerialData(); // The buffer holding the received serial data.


var port = new SerialPort(PORT, {
	baudRate: 115200,
	autoOpen: false
});

/**
 * Maintain received serial data.
 * 
 * We are receiving serial data asynchronosly from the serial processor.  This means that when data arrives
 * on the UART, it is passed to an instance of this object for accumulation.  The data is passed in through a
 * call to append().
 */
function SerialData() {
	var serialDataBuffer;        // The serial data we are accumulating.
	var resolveMoreData = null;  // A resolve function for a promise that will be resolved when more data has arrived.


	/**
	 * Purge and reset any existing unprocessed serial data.
	 */
	function empty() {
		serialDataBuffer = Buffer.allocUnsafe(0);
	} // empty


	/**
	 * Append data received from the serial port into our buffer.
	 * @param {Buffer} data The data received from our serial port.
	 * @returns Nothing.
	 */
	function append(data) {
		console.log("SerialData.append:\n%s", hexy.hexy(data));
		serialDataBuffer = Buffer.concat([serialDataBuffer, data]);      // Append the new data into the accumulating serial buffer.
		if (resolveMoreData != null) {                                   // If we have a promise resolution that is waiting for more data ..
			resolveMoreData();                                           // resolve the promise.
			resolveModeData = null;
		}
	} // append


	/**
	 * Create a promise that is resolved when we have more data.
	 * @returns A promise that is resolved when we have more data.
	 */
	async function moreData() {
		return new Promise((resolveA, rejectA) => {
			resolveMoreData = resolveA;                                  // The promise is resolved when append is called to provide more data.
		});
	} // moreData


	/**
	 * Get the next SLIP data packet.
	 * Assume that the serial data received is a sequence of packets.  This function returns a promise that
	 * is resolved when a packet is available for processing.
	 */
	async function getNextDataPacket() {
		var p = new Promise(async (resolve, reject) => {
			console.log("getNextDataPacket>>");
			while(true) {                                                                   // Keep looping until we have a data packet.
				if (DataPacket().containsSLIPPacket(serialDataBuffer)) {                    // Does the buffer now contain a SLIP packet?
					var newDataPacket = DataPacket();                                       // Create a new data packet
					serialDataBuffer = newDataPacket.setSLIPPacket(serialDataBuffer);       // Load the data packet with the raw serial data.
					console.log("Got a new data packet: %s\n%s",
						newDataPacket.toString(), hexy.hexy(newDataPacket.getData()));
					resolve(newDataPacket);
					return;
				}
				await moreData();                                                // We didn't find a data packet in our data, so wait for more data.
			} // We will loop forever waiting for packets.
		});
		return p;                                                                // Return the promise that is resolved when we have a data packet.
	} // getNextDataPacket


	empty(); // Initialize / empty the serial data.
	return {
		"append":            append,
		"empty":             empty,
		"getNextDataPacket": getNextDataPacket
	}
} // SerialData


/**
 * A data packet encapsulates a command or response.
 */
function DataPacket() {
	var dataPacket = Buffer.allocUnsafe(0);

	/**
	 * Examine the data buffer looking for a slip trailer (0xc0).  If found, we return the index
	 * at which it was found.  If not found, we return -1.
	 * @param {Buffer} data The data in which we are looking for the slip trailer.
	 * @returns The original of the slip trailer or -1 if not found.
	 */
	function getSLIPTrailer(data) {
		for (i=1; i<data.length; i++) {                                       // Walk through all the bytes in the data supplied.
			if (data[i] == 0xc0) {                                            // If we find a slip trailer, return its index.
				return i;
			}
		}
		return -1;                                                           // If we did not find a slip trailer, return -1.
	} // getSLIPTrailer


	/**
	 * Examine the data.  Return true if we have a packet and false otherwise.
	 * @param {Buffer} data The data we are to examine to see if we have a whole packet.
	 * @returns true if we have a whole packet.
	 */
	function containsSLIPPacket(data) {
		return getSLIPTrailer(data) > 0;
	} // containsSLIPPacket

	
	/**
	 * Process the data passed in that is SLIP encoded and build a parsed data packet.
	 * @param {Buffer} data The data to build a packet from.  This will be raw data received from UART.
	 * @returns The unprocessed data.
	 */
	function setSLIPPacket(data) {
		console.log("setSLIPPacket:\n%s", hexy.hexy(data));
		assert(data);                          // Assert if we have been presented no data.
		assert(data.length > 0);               // Assert that the data length is not 0.
		assert(data[0] == 0xc0);               // Assert if the first bytes isn't a SLIP marker.
         
		var end = getSLIPTrailer(data);        // Get the end marker for the SLIP packet.
		if (end == -1) {
			throw "No SLIP trailer in data we are to process.";
		}
		assert(end > 0);

		dataPacket = Buffer.allocUnsafe(data.length);                     // Our data will be no MORE (but likely less) than data.length bytes in size.
		for (var i=1, j=0; i<end; i++, j++) {                             // Remove escape sequences.
			if (data[i] == 0xdb && data[i+1] == 0xdc) {                   // 0xdb 0xdc -> 0xc0
				dataPacket[j] = 0xc0;
				i++
			} else if (data[i] == 0xdb && data[i+1] == 0xdd) {            // 0xdb 0xdd -> 0xdb
				dataPacket[j] = 0xdb;
				i++;
			} else {
				dataPacket[j] = data[i]
			}
		} // End loop over data.

		// We now have a buffer (dataPacket) that contains our decoded data but will be too large.  However, j contains the number of bytes we want.
		var temp = Buffer.allocUnsafe(j);
		dataPacket.copy(temp, 0, 0, j);                                  // Copy dataPacket[0..j-1] -> temp
		dataPacket = temp;                                               // Set datapacket to the shrunk buffer.

		// We have now processed what we wanted from the incoming data and now we must return the unused data.
		i++;
		var unprocessedLength = data.length - i;
		temp = Buffer.allocUnsafe(unprocessedLength);
		data.copy(temp, 0, i);                                           // copy data[i..] -> temp

		console.log("setSLIPPacket: New data has length: %d, returning unused buffer of size: %d", dataPacket.length, temp.length);
		if (temp.length > 0) {
			console.log("Unused:\n%s", hexy.hexy(temp));
		}
		return temp;
	} // setSLIPPacket


	/**
	 * Internally we have a dataPacket.  We want to return a SLIP encoded buffer from that packet.
	 * @returns A SLIP encoded representation of our data Packet.
	 */
	function getSLIPPacket() {
		var temp = Buffer.allocUnsafe(dataPacket.length * 2);
		var i = 0;  // The variable called "i" will be the current byte being written into the slip encoded output.
		temp[i] = 0xc0;
		i++;
		for (j=0; j<dataPacket.length; j++, i++) {      // If the current byte is 0xc0 escape to 0xdb 0xdc
			if (dataPacket[j] == 0xc0) {
				temp[i] = 0xdb;
				i++;
				temp[i] = 0xdc;
 			} else if (dataPacket[j] == 0xdb) {         // If the current byte is 0xdb escape to 0xdb 0xdd
				temp[i] = 0xdb;
				i++;
				temp[i] = 0xdd;
			} else {
				temp[i] = dataPacket[j];
			}
		}
		var temp2 = Buffer.allocUnsafe(i+1);
		temp.copy(temp2, 0, 0, i);   // Copy temp[0..i-1] -> temp2
		temp2[i] = 0xc0;             // Terminate with a slip trailder.
		return temp2;
	} // getSLIPPacket

	/**
	 * Set the data.
	 * @param {*} data 
	 */
	function setData(data) {
		dataPacket = data;
	}

	function getData() {
		return dataPacket;
	}

	function toString() {
		var result;
		if (dataPacket[0] == 0) {
			result  = "Request: ";
			result += "command: " + commandToString(dataPacket[1]);
			result += ", size: " + dataPacket.readUInt16LE(2);
			result += ", calculatedSize: " + (dataPacket.length - 8);
		} else if (dataPacket[0] == 1) {
			result  = "Response: ";
			result += "command: " + commandToString(dataPacket[1]);
			result += ", size: " + dataPacket.readUInt16LE(2);
			result += ", calculatedSize: " + (dataPacket.length - 8);
			result += ", value: " + dataPacket.readUInt32LE(4).toString(16);
			// For the ESP32
			// Size - 4 = status
			// Size - 3 = error
			result += ", status: " + dataPacket[dataPacket.length-2];
			result += ", errorCode: " + dataPacket[dataPacket.length-1];
		} else {
			result = "Unknown command: " + dataPacket[0];
		}
		return result;
	} // toString

	return {
		"containsSLIPPacket": containsSLIPPacket,   // Does the data passed in define contain a SLIP packet?
		"getSLIPPacket":      getSLIPPacket,
		"setSLIPPacket":      setSLIPPacket,
		"getData":            getData,
		"setData":            setData,
		"toString":           toString
	} // End of return
} // DataPacket


/**
 * Convert a command code to a string representation.
 * @param {String} command The command to be converted.
 * @returns A string representation of the command.
 */
function commandToString(command) {
	switch(command) {
		case COMMAND_FLASH_BEGIN:
			return "COMMAND_FLASH_BEGIN";	
		case COMMAND_FLASH_DATA:
			return "COMMAND_FLASH_DATA";
		case COMMAND_FLASH_END:
			return "COMMAND_FLASH_END";	
		case COMMAND_MEM_BEGIN:
			return "COMMAND_MEM_BEGIN";
		case COMMAND_MEM_END:
			return "COMMAND_MEM_END";
		case COMMAND_MEM_DATA:
			return "COMMAND_MEM_DATA";									
		case COMMAND_SYNC:
			return "COMMAND_SYNC";
		case COMMAND_WRITE_REG:
			return "COMMAND_WRITE_REG";			
		case COMMAND_READ_REG:
			return "COMMAND_READ_REG";
		case COMMAND_SPI_SET_PARAMS:
			return "COMMAND_SPI_SET_PARAMS";
		case COMMAND_SPI_FLASH_MD5:
			return "COMMAND_SPI_FLASH_MD5";			
	}			
	return "Unknown command: " + command;
} // commandToString

	
function setFlags(options) {
	return new Promise((resolve, reject) => {
		port.set(options, () => {
			resolve();
		});
	});
} // setFlags


function delay(msecs) {
	return new Promise((resolve, reject) => {
		setTimeout(() => {
			resolve();
		}, msecs);
	});
} // delay


/**
 * Write the command header
 * @param command             // The ESP32 flash loader command
 * @param data                // The data to send.
 * @param checksum            // [Optional] The checksum value
 * @returns
 */
async function buildAndSendRequest(command, data, checksum = 0) {
	console.log("buildAndSendRequest: %s, checksum: 0x%s, dataLength: %d", commandToString(command), checksum.toString(16), data.length);
	var buf = Buffer.allocUnsafe(8 + data.length);     // Create a buffer used to hold the command.
	buf.writeUInt8(0,              0);                 // Flag as a request  [0]
	buf.writeUInt8(command,        1);                 // Write the command  [1]
	buf.writeUInt16LE(data.length, 2);                 // Write the size     [2-3]
	buf.writeUInt32LE(checksum,    4);                 // Write the checksum [4-7]
	data.copy(buf, 8);                                 // Copy data[0..] to buf[8..]
	var dataPacket = DataPacket();                     // Create a new data packet object
	dataPacket.setData(buf);                           // Populate the data packet
	await portWrite(dataPacket.getSLIPPacket());       // Send the data packet to the ESP32.
} // buildAndSendRequest


/**
 * Promise to flush all pending transmitting data before resolution.
 * @returns A promise that is resolved when the transmission data has all been sent.
 */
async function drain() {
	return new Promise((resolve, reject) => {
		port.drain(() => {resolve()});
	});
} // drain


/**
 * Promise to discard all un-transmitted output data and discard all unread input data.
 * @returns A promise that is resolved when input/output data is discarded.
 */
function flush() {
	return;
	console.log("Flushing...");
	return new Promise((resolve, reject) => {
		port.flush(()=>{resolve()});
	});
} // flush

// We send a UART command request.  The thinking is that we will now start to receive a UART command response.
// The response won't arrive as a unit but will instead be dribbled in over time asynchronously.  As such, we need to maintain
// state to process it.  A response is composed of a response header and response payload.  The response header (as seen by
// UART) will contain:
// 0xC0                - SLIP header
// 0x01                - Direction.  Should be 0x01 for a response.
// 0x??                - Command
// 0x?? 0x??           - Size of payload data.
// 0x?? 0x?? 0x?? 0x?? - Value.  Used by some commands (NOT ALL)!!
// 0x?? ...            - Data as indicated by the size command.
// 0x?? 0x?? 0x?? 0x?? - Outcome
// 0xC0                - SLIP trailer
function ProcessResponse() {
	var command;
	var data;
	var dataSize;
	var value;
	var status;
	var errorCode;
	var resolve;
	var reject = null;
	var promise = null;

	var timeoutId;


	async function getResponse() {
		console.log(">> ProcessResponse.getResponse()")

		promise = new Promise(async (resolveA, rejectA) => {
			resolve = resolveA;
			reject  = rejectA;
			timeoutId = setTimeout(()=>{
				console.log("getResponse timeout!");
				reject();
			}, 1000);
			var dp = await serialData.getNextDataPacket();
			console.log(" .. getResponse ... we think we have a new datapacket!");
			processPacket(dp.getData());
			resolve();
		});
		return promise;
	}

	function processPacket(inputData) {
		assert(inputData);
		assert(inputData.length > 0);
		console.log("Received a packet, length %d:\n%s", inputData.length, hexy.hexy(inputData));
		// 0 - direction
		// 1 - command
		// [2-3] - size
		// [4-7] - value
		// [8-(length-3)] - data
		// [length-2] - status
		// [length-1] - error code
		if (inputData.readUInt8(0) != 0x01) {
			throw "Expected direction to be 0x01 for a response.";
		}
		command  = inputData.readUInt8(1);
		dataSize = inputData.readUInt16LE(2);
		value    = inputData.readUInt32LE(4);
		data     = Buffer.allocUnsafe(inputData.length - 8 - 2);
		inputData.copy(data, 0, 8, inputData.length - 2);
		status   = inputData.readUInt8(inputData.length - 2);
		erorCode = inputData.readUInt8(inputData.length - 1);
		clearTimeout(timeoutId);
	} // processPacket

	return {
		getResponse:   getResponse,
		getData: function() {
			return data;
		}
	};
} // ProcessResponse


/**
 * Send a SYNC command and wait for the response.
 */
async function doSync() {
	console.log("Sending sync");
	var p = new Promise(async function(resolve, reject) {
		await flush();
		var buf = Buffer.allocUnsafe(36); // The sync message is 0x07 0x07 0x12 0x20 0x55 (repeated 32 times)
		buf.writeUInt8(0x07, 0);
		buf.writeUInt8(0x07, 1);
		buf.writeUInt8(0x12, 2);
		buf.writeUInt8(0x20, 3);
		for (var i=4; i<36; i++) {
			buf.writeUInt8(0x55, i);
		}
		await buildAndSendRequest(COMMAND_SYNC, buf);
		console.log("response: " + response);
		try {
			await response.getResponse();
			resolve();
		}
		catch(e) {
			console.log("Failed to get SYNC command response: " + e);
			reject();
		}		
	});
	return p;
} // doSync


/**
 * Calculate the checksum of the data.
 * The current algorithm is the seed 0xEF XORed with each of the bytes of data.
 * @param {Buffer} data The data against which the checksum is to be calculated.
 * @returns The checksum value.
 */
function calculateChecksum(data) {
	var checksum = 0xef;
	for (var i=0; i<data.length; i++) {
		checksum ^= data[i];
	}
	return checksum;
} // calculateChecksum

/**
 * A class that encapsulates memory transmission to ESP32 RAM.
 */
function MemCommands() {
	const MEM_PACKET_SIZE = 0x1800;
	var sequenceNumber;
	var totalSize;
	var sizeWritten;
	
	function memBegin(size, address) {
		console.log("Mem Begin: size: %d, address: 0x%s", size, address.toString(16));
		var p = new Promise(async function (resolve, reject) {
			console.log("+-----------+");
			console.log("| MEM_BEGIN |");
			console.log("+-----------+");
			await flush();
			var buf = Buffer.allocUnsafe(4*4);
			buf.writeUInt32LE(size,                            0);                               // Size of data to be sent.
			buf.writeUInt32LE(Math.ceil(size/MEM_PACKET_SIZE), 4);                               // Number of data packets to be sent.
			buf.writeUInt32LE(MEM_PACKET_SIZE,                 8);                               // Size of each data packet.
			buf.writeUInt32LE(address,                        12);                               // Offset in memory to start writing.
			await buildAndSendRequest(COMMAND_MEM_BEGIN, buf);
			await response.getResponse();
			resolve();
		});
		sequenceNumber = 0;                                                                      // The sequence number of the next MEM_DATA transmission.
		totalSize      = size;                                                                   // The size of the data we are going to be sending.
		sizeWritten    = 0;                                                                      // The size of the data we have written so far.
		
		return p;
	} // memBegin
	
	function memEnd(executeFlag, entrypointAddress) {
		console.log("+---------+");
		console.log("| MEM_END |");
		console.log("+---------+");
		console.log(">> memEnd: executeFlag: %d, entrypointAddress: 0x%s", executeFlag, entrypointAddress.toString(16));
		var p = new Promise(async function (resolve, reject) {
			await flush();                                                                       // Discard any pending input or output.
			var buf = Buffer.allocUnsafe(2*4);                                                   // Allocate the payload which is 2 * 32bit words
			buf.writeUInt32LE(executeFlag,       0);                                             // Execute flag.
			buf.writeUInt32LE(entrypointAddress, 4);                                             // Entry point address.
			await buildAndSendRequest(COMMAND_MEM_END, buf);
			await response.getResponse();                                                        // Wait for a response.
			resolve();                                                                           // Resolve the promise.
		});
		return p;		
	} // memEnd
	
	/**
	 * Determine if we have sent all the data for the given MEM_BEGIN transaction.
	 * @returns True if we have sent all the data we need to send.
	 */
	function allDataSent() {
		// We maintain state in a number of variables.  The variable called sizeWritten is the number of bytes that we have written to
		// the ESP32.  The variable called totalSize is the number of bytes that we expect to write.  The notion of all data sent
		// will occur when the sizeWritten is equal to totalSize.
		return sizeWritten >= totalSize;
	} // allDataSent


	/**
	 * Write a MEM_DATA command.
	 * This command should follow a MEM_BEGIN command.  If transmits a unit of data that is written directly into
	 * the ESP32 RAM.  The address in RAM is supplied by the MEM_BEGIN command and previous MEM_DATA calls.
	 * @param {Buffer} data 
	 */
	async function memData(data) {
		console.log(">> memData: sequenceId: %d", sequenceNumber);
		var sizeRemaining  = totalSize - sizeWritten;                                            // Calculate the amount of data still expected.
		var offsetIntoData = sequenceNumber * MEM_PACKET_SIZE;                                   // Calculate the offset into the data from which to send.
		var sizeToWrite    = sizeRemaining > MEM_PACKET_SIZE?MEM_PACKET_SIZE:sizeRemaining;      // Calculate how large this unit should be.
		var buf2           = Buffer.allocUnsafe(4*4 + data.length);                              // Allocate storage for the command preamble.
		buf2.writeUInt32LE(sizeToWrite,    0);                                                   // Size of data to be sent.
		buf2.writeUInt32LE(sequenceNumber, 4);                                                   // Current sequence number.
		buf2.writeUInt32LE(0,              8);                                                   // Zero.
		buf2.writeUInt32LE(0,             12);                                                   // Zero.
		data.copy(buf2, 16);
		//var buf = Buffer.concat([buf2, data]);
		//console.log("Here is the data we are sending:\n%s", hexy.hexy(buf));
		
		var p = new Promise(async function(resolve, reject) {
			console.log("+----------+");
			console.log("| MEM_DATA |");
			console.log("+----------+");
			await flush();                                                                       // Discard any untransmitted and unreceived data.                                                                    // End the SLIP communication.
			await buildAndSendRequest(COMMAND_MEM_DATA, buf2, calculateChecksum(data));			
			sequenceNumber ++;                                                                   // Increment the sequence number.
			sizeWritten += sizeToWrite;                                                          // Updae the size of the data that we have now written.                                                                    // Ensure that all the transmitted data is now outbound.
			await response.getResponse();                                                        // Await a response.
			resolve();                                                                           // Indicate that the command has been completed.
		});	
		return p;	
	}; // memData

	
	/**
	 * Transmit a buffer of data to a given address within ESP32 RAM.
	 * @param {Buffer} data The data to be written into ESP32 RAM.
	 * @param {integer} address The address that data will be written to.
	 */
	async function send(data, address) {
		// If the data is not 32bit aligned, expand to 32 bits.
		if (data.length % 4 != 0) {
			let buf = Buffer.allocUnsafe((data.length + 4) & 0xFFFFFFFC).fill(0xff);
			data.copy(buf);
			data = buf;
		}
		console.log(">> send data of size: %d, to RAM address 0x%s", data.length, address.toString(16));
		await memBegin(data.length, address);
		while(!allDataSent()) {
			await memData(data);
		}
	}; // send

	
	return {
		"send": send,
		"end":  memEnd
	};
} // MemCommands


function portWrite(data) {
	console.log("Writing data down port, length: %d", data.length)
	return new Promise((resolve, reject) => {
		port.write(data, async ()=> {
			await drain;
			resolve()
		});
	});
} // portWrite


/**
 * Process the handling of flash commands.
 */
function FlashCommands() {
	var sequenceNumber;                  // The current sequence number.
	var totalSize;                       // The total number of bytes we expect to send.
	var sizeWritten;                     // The number of bytes we have written so far.

	const FLASH_PACKET_SIZE = 0x4000; // 16K = 16384 bytes


	async function flashBegin(size, address) {
		console.log("Flash Begin: size: %d, address: 0x%s", size, address.toString(16));
		var p = new Promise(async function (resolve, reject) {
			console.log("+-------------+");
			console.log("| FLASH_BEGIN |");
			console.log("+-------------+");
			await flush();
			var buf = Buffer.allocUnsafe(4*4);
			buf.writeUInt32LE(size,                              0);                          // Size of data to be sent/erased.
			buf.writeUInt32LE(Math.ceil(size/FLASH_PACKET_SIZE), 4);                             // Number of data packets to be sent.
			buf.writeUInt32LE(FLASH_PACKET_SIZE,                 8);                             // Size of each data packet.
			buf.writeUInt32LE(address,                           12);                            // Offset in memory to start writing.
			await buildAndSendRequest(COMMAND_FLASH_BEGIN, buf);			
			await response.getResponse();			
			resolve();
		});		
		return p;
	} // flashBegin	


	async function flashData(data, sequenceNumber) {
		console.log(">> flashData: sequenceId: %d, size: %d", sequenceNumber, data.length);
		if (data.length < FLASH_PACKET_SIZE) {
			var temp = Buffer.alloc(FLASH_PACKET_SIZE, 0xff);
			data.copy(temp);
			data = temp;
		}
		var buf2 = Buffer.alloc(4*4 + data.length);                                        // Allocate storage for the command preamble.
		buf2.writeUInt32LE(data.length,    0);                                             // Size of data to be sent.
		buf2.writeUInt32LE(sequenceNumber, 4);                                             // Current sequence number.
		buf2.writeUInt32LE(0,              8);
		buf2.writeUInt32LE(0,              12);				
		data.copy(buf2, 16);
		//var buf = Buffer.concat([buf2, data]);
		//console.log("Here is the data we are sending:\n%s", hexy.hexy(buf));
		
		var p = new Promise(async function(resolve, reject) {
			console.log("+------------+");
			console.log("| FLASH_DATA |");
			console.log("+------------+");
			await flush();                                                                       // Discard any untransmitted and unreceived data.                                      
			await buildAndSendRequest(COMMAND_FLASH_DATA, buf2, calculateChecksum(data));				
			await response.getResponse();                                                        // Await a response.
			resolve();                                                                           // Indicate that the command has been completed.
		});	
		return p;	
	}; // flashData


	/**
	 * Send the FLASH_END command.
	 * @param {Integer} command 
	 * @returns A promise that is resolved when the FLASH_END command completes.
	 */
	function flashEnd(command) {
		console.log("+-----------+");
		console.log("| FLASH_END |");
		console.log("+-----------+");
		console.log(">> memEnd: command: %d", command);
		var p = new Promise(async function (resolve, reject) {
			await flush();                                                                       // Discard any pending input or output.
			var buf = Buffer.allocUnsafe(4);                                                     // Allocate the payload which is one 32bit word.
			buf.writeUInt32LE(command, 0);                                                       // Execute flag.
			await buildAndSendRequest(COMMAND_FLASH_END, buf);				
			await response.getResponse();                                                        // Wait for a response.
			resolve();                                                                           // Resolve the promise.
		});
		return p;		
	} // flashEnd

	/**
	 * Transmit a buffer of data to a given address within ESP32 FLASH.
	 * @param {Buffer} data The data to be written into ESP32 FLASH.
	 * @param {integer} address The address that data will be written to.
	 */
	async function send(data, address) {
		return new Promise(async (resolve, reject) => {
			// If the data is not 32bit aligned, expand to 32 bits.
			if (data.length % 4 != 0) {
				let buf = Buffer.allocUnsafe((data.length + 4) & 0xFFFFFFFC).fill(0xff);
				data.copy(buf);
				data = buf;
			}
			console.log(">> send flash data of size: %d, to flash address 0x%s", data.length, address.toString(16));
			await flashBegin(data.length, address);

			sequenceNumber = 0;                                                                      // The sequence number of the next FLASH_DATA transmission.
			totalSize      = data.length;                                                            // The size of the data we are going to be sending.
			sizeWritten    = 0;                                                                      // The size of the data we have written so far.	

			while(sizeWritten < totalSize) {
				var sizeRemaining  = totalSize - sizeWritten;                                            // Calculate the amount of data still expected.
				var offsetIntoData = sequenceNumber * FLASH_PACKET_SIZE;                                 // Calculate the offset into the data from which to send.
				var sizeToWrite    = sizeRemaining > FLASH_PACKET_SIZE?FLASH_PACKET_SIZE:sizeRemaining;  // Calculate how large this unit should be.
				var tempBuf        = Buffer.allocUnsafe(sizeToWrite);
				data.copy(tempBuf, 0, offsetIntoData, offsetIntoData+sizeToWrite);
				await flashData(tempBuf, sequenceNumber);
				sequenceNumber++;
				sizeWritten += sizeToWrite;
			}
			await flashEnd(1);
			resolve();
		});
	}; // send

	return {
		"send": send,
		"end":  flashEnd
	};

} // FlashCommands


/**
 * Upload the flasher application.
 * @returns A Promise that is fulfilled when the flasher has been uploaded.
 */
async function uploadFlasher() {
	// We read a JSON file that has been prepared to contain the flasher application.  This JSON contains:
	// {
	//   textAddress: <String> - A string representation (hex) of the memory address into which the text data should be loaded.
	//   dataAddress: <String> - A string representation (hex) of the memory address into which the data data should be loaded.
	//   textData:    <String> - A base64 encoded representation of the text data.
	//   dataData:    <String> - A base64 encoded representation of the data data
	//   entryPoint:  <String> - A string representation of the entry point (hex) for execution.
	// }
	//	
	return new Promise(async (resolve, reject)=>{
		console.log(">> uploadFlasher");
		// Load the flasher from the local file.
		var flasherDataRaw = fs.readFileSync("esptool/flasher_stub/build/flasher_stub.json", "utf8");
		var flasherData    = JSON.parse(flasherDataRaw);

		flasherData.textData = Buffer.from(flasherData.textData, "base64");
		flasherData.dataData = Buffer.from(flasherData.dataData, "base64");
		console.log("Text data length: %d, Data data length: %d", flasherData.textData.length, flasherData.dataData.length);
	
		await MemCommands().send(flasherData.textData, parseInt(flasherData.textAddress, 16));
		await MemCommands().send(flasherData.dataData, parseInt(flasherData.dataAddress, 16));
		await MemCommands().end(0, parseInt(flasherData.entryPoint, 16));
		console.log("<< uploadFlasher complete");
		resolve();
	}); // End of promise
} // uploadFlasher


/**
 * Write a value to an ESP32 register defined.
 * @param {Integer} address The address of the register.
 * @param {Integer} value The value to be written to the register.
 * @returns A promise that is fulfilled when the register has been written.
 */
function writeReg(address, value) {
	console.log("+-----------+");
	console.log("| WRITE_REG |");
	console.log("+-----------+");	
	console.log(">> Writing register:  Command: 0x%s, Address: 0x%s, value: 0x%s",
		COMMAND_WRITE_REG.toString(16), address.toString(16), value.toString(16));
	var p = new Promise(async function(resolve, reject) {
		await flush();
		var buf = Buffer.allocUnsafe(4*4);
		buf.writeUInt32LE(address,    0);
		buf.writeUInt32LE(value,      4);		
		buf.writeUInt32LE(0xffffffff, 8);    // Mask
		buf.writeUInt32LE(0,          12);   // Delay
		await buildAndSendRequest(COMMAND_WRITE_REG, buf);				
		await response.getResponse();
		resolve();
	});
	return p;
} // writeReg


/**
 * Set the SPI configuration parameters.
 * @param {Integer} size The size of SPI flash.
 * @returns A promise that is fulfilled when the SPI parameters have been set.
 */
function spiSetParams(size) {
	console.log("+----------------+");
	console.log("| SPI_SET_PARAMS |");
	console.log("+----------------+");
	console.log(">> SPI Set Params:  Command: 0x%s, size:%d",
		COMMAND_SPI_SET_PARAMS.toString(16), size);
	var p = new Promise(async function(resolve, reject) {
		await flush();
		var buf = Buffer.allocUnsafe(6*4);
		buf.writeUInt32LE(0,       0);    // id
		buf.writeUInt32LE(size,    4);	  // Total size	
		buf.writeUInt32LE(64*1024, 8);    // block size
		buf.writeUInt32LE(4*1024,  12);   // sector size
		buf.writeUInt32LE(256,     16);   // page size
		buf.writeUInt32LE(0xffff,  20);   // status mask				
		await buildAndSendRequest(COMMAND_SPI_SET_PARAMS, buf);				
		await response.getResponse();
		resolve();
	});
	return p;
} // spiSetParams


/**
 * Erase the flash memory.
 * @returns A promise that is fulfilled when the flash memory has been erased.
 */
function eraseFlash() {
	console.log("+-------------+");
	console.log("| ERASE_FLASH |");
	console.log("+-------------+");
	console.log(">> Erase Flash:  Command: 0x%s",	COMMAND_ERASE_FLASH.toString(16));
	var p = new Promise(async function(resolve, reject) {
		await flush();
		var buf = Buffer.allocUnsafe(0);			
		await buildAndSendRequest(COMMAND_ERASE_FLASH, buf);				
		await response.getResponse();
		resolve();
	});
	return p;
} // spiSetParams


/**
 * Read an ESP32 register.
 * @param {Integer} address The address of the register to be read.
 * @returns The result of reading the register.
 */
function readReg(address) {
	console.log(">> Reading register:  Command: 0x%s, Address: 0x%s", COMMAND_READ_REG.toString(16), address.toString(16));
	var p = new Promise(async function(resolve, reject) {
		await flush();
		var buf = Buffer.allocUnsafe(4);
		buf.writeUInt32LE(address, 0);
		await buildAndSendRequest(COMMAND_READ_REG, buf);		
		var result = await response.getResponse();
		console.log("readReg: %O", result);
		resolve(result);
	});
	return p;
} // readReg



/**
 * Get the MD5 hash of an area of flash memory.
 * @param {Integer} address The address of the flash memory.
 * @param {Integer} size The size of the flasg memory.
 * @returns The MD5 hash value.
 */
function spiFlashMD5(address, size) {
	console.log("+---------------+");
	console.log("| SPI_FLASH_MD5 |");
	console.log("+---------------+");
	console.log(">> spiFlashMD5:  Command: 0x%s, Address: 0x%s, Size: %d", COMMAND_SPI_FLASH_MD5.toString(16), address.toString(16), size);
	var p = new Promise(async function(resolve, reject) {
		await flush();
		var buf = Buffer.allocUnsafe(4 * 4);
		buf.writeUInt32LE(address, 0);
		buf.writeUInt32LE(size,    4);
		buf.writeUInt32LE(0,       8);
		buf.writeUInt32LE(0,       12);						
		await buildAndSendRequest(COMMAND_SPI_FLASH_MD5, buf);			
		await response.getResponse();
		console.log("spiFlashMD5 result:\n%s", hexy.hexy(response.getData()));
		resolve(response.getData());
	});
	return p;
} // spiFlashMD5


/**
 * Read the value of an eFuse.
 * @param {Integer} fuseNumber 
 * @returns The value of the eFuse.
 */
function readEfuse(fuseNumber) {
	console.log(">> readEFuse: %d", fuseNumber);
	return readReg(EFUSE_REG_BASE + (4 * fuseNumber));
} // readEfuse


/**
 * The ESP32 needs to be in a "download" mode in order to process commands.  For many boards, a sequence of
 * DTR/RTS UART twiddles can be sends to achieve that goal.  This function performs that task.
 * 
 * @returns A Promise that is fulfilled when the ESP32 is in download mode.
 */
function enterDownloadMode() {
	console.log(">> enterDownloadMode: Enter download mode, esp32r0Delay = " + esp32r0Delay);
	return new Promise((resolve, reject) => {
		setFlags({ dtr: false, rts: true	}).then(()=>{
			return delay(100);
		}).then(() => {
			if (esp32r0Delay) {
				return delay(1200);
			}
			return Promise.resolve();
		}).then(()=> {
			return setFlags({ dtr: true, rts: false });
		}).then(() => {
			if (esp32r0Delay) {
				return delay(400);
			}
			return Promise.resolve();
		}).then(()=> {
			return delay(50);
		}).then(()=> {
			return setFlags({ dtr: false, rts: false });
		}).then(() => {
			resolve();
		});
	});
} // enterDownloadMode


/**
 * Flash the content of the file supplied by filename into ESP32 flash specified by the address.
 * @param {Integer} address The address in flash to write the file.
 * @param {String} fileName The name of the file to read from.
 */
async function flashFile(address, fileName) {
	console.log("Flashing file %s to address 0x%s", fileName, address.toString(16));
	var fileData = fs.readFileSync(fileName);
	console.log("Read file ... size is %d, md5: %s", fileData.length, md5(fileData));
	await FlashCommands().send(fileData, address);
	console.log("Flash File complete");
	return fileData.length;
} // flashFile


async function sleep(interval) {
	return new Promise((resolve) => {
		setTimeout(() => {resolve()}, interval);
	});
}


console.log("Start!");
if (argv._.length%2 != 0) {
	console.log("Wrong number of args");
	return;
}
response = ProcessResponse();
console.log("Response created: " + response);
port.open(async (err)=>{
	if (err) {
		console.log("Error opening: " + err);
		return;
	}
	console.log("Open!");
	await enterDownloadMode();
	//var p = Promise.resolve(); 
	console.log("We should now be in ESP32 download mode!");
	serialData.empty();
	while(1) {
		try {
			await doSync();
			break;
		}
		catch(e) {
			console.log(e);
		}
	}
	await sleep(1000);
	serialData.empty();
	console.log("+----------------+")
	console.log("| Sync Complete! |");
	console.log("+----------------+")	

	await readEfuse(3);
	console.log("Read fuse completed!");
	await uploadFlasher();
	serialData.empty();
	console.log("Flasher upload completed and running!");
	/*
	await writeReg(0x6000202c, 0x00000017);
	await writeReg(0x6000201c, 0x90000000);	
	await writeReg(0x60002024, 0x9f000070);	
	await writeReg(0x60002080, 0x00000000);	
	await writeReg(0x60002000, 0x00040000);	
	await writeReg(0x6000201c, 0x80000040);
	await writeReg(0x60002024, 0x00000000);	
	*/	
	await spiSetParams(0x400000);

	// Walk through each of the files presented and upload them.  The input on the command line should be of the form:
	// address file [address file]*

	for (var i=0; i<argv._.length;) {          // While there are more files to process.
		var address = parseInt(argv._[i]);     // Obtain the address.
		i++;
		var fileName = argv._[i];              // Obtain the file name.
		i++;
		var size = await flashFile(address, fileName);          // Invoke the processor to upload the file into flash.
		console.log("File size written: %d", size);
		await spiFlashMD5(address, size);
	} // End of process each file.
	

	console.log("<< on port open");
});

port.on("data", (data) => {
	serialData.append(data);  // Append the new data into the serial buffer.
});

console.log("Init done");
