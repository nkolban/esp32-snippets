/*
 * FTPServer.cpp
 *
 *  Created on: May 6, 2018
 *      Author: kolban
 */

#include "FTPServer.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <esp_log.h>

static const char* LOG_TAG = "FTPServer";

// trim from start (in place)
static void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
} // ltrim


// trim from end (in place)
static void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
} // rtrim


// trim from both ends (in place)
static void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
} // trim


FTPServer::FTPServer() {
	ESP_LOGD(LOG_TAG,">> FTPServer()");

	m_dataSocket    = -1;
	m_clientSocket  = -1;
	m_dataPort      = -1;
	m_dataIp        = -1;
	m_passiveSocket = -1;
	m_serverSocket  = -1;

	m_callbacks       = nullptr;
	m_isPassive       = false;
	m_isImage         = true;
	m_chunkSize       = 4096;
	m_port            = 21; // The default Server-PI port
	m_loginRequired   = false;
	m_isAuthenticated = false;
	m_userid = "";
	m_password = "";

	ESP_LOGD(LOG_TAG,"<< FTPServer()");
} // FTPServer#FTPServer


FTPServer::~FTPServer() {
	// Nothing to do here by default.
} // FTPServer#~FTPServer


/**
 * Close the connection to the FTP client.
 */
void FTPServer::closeConnection() {
	ESP_LOGD(LOG_TAG,">> closeConnection");
	close(m_clientSocket);
	ESP_LOGD(LOG_TAG,"<< closeConnection");
} // FTPServer#closeConnection


/**
 * Close a previously opened data connection.
 */
void FTPServer::closeData() {
	ESP_LOGD(LOG_TAG,">> closeData");
	close(m_dataSocket);
	m_dataSocket = -1;
	ESP_LOGD(LOG_TAG,"<< closeData");
} // FTPServer#closeData


/**
 * Close the passive listening socket that was opened by listenPassive.
 */
void FTPServer::closePassive() {
	ESP_LOGD(LOG_TAG,">> closePassive");
	close(m_passiveSocket);
	m_passiveSocket = -1;
	ESP_LOGD(LOG_TAG, "<< closePassive");
} // FTPServer#closePassive


/**
 * Retrieve the current directory.
 */
/* STATIC */ std::string FTPServer::getCurrentDirectory() {
	char maxDirectory[256];
	std::string currentDirectory = getcwd(maxDirectory, sizeof(maxDirectory));
	return currentDirectory;
} // FTPServer#getCurrentDirectory


/**
 * Create a listening socket for the new passive connection.
 * @return a String for the passive parameters.
 */
std::string FTPServer::listenPassive() {
	ESP_LOGD(LOG_TAG, ">> listenPassive");

	m_passiveSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_passiveSocket == -1) {
		ESP_LOGD(LOG_TAG, "socket: %s", strerror(errno));
	}

	struct sockaddr_in clientAddrInfo;
	unsigned int addrInfoSize = sizeof(clientAddrInfo);
	getsockname(m_clientSocket, (struct sockaddr*) &clientAddrInfo, &addrInfoSize);

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(0);
	int rc = bind(m_passiveSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	if (rc == -1) {
		ESP_LOGD(LOG_TAG, "bind: %s", strerror(errno));
	}

	rc = listen(m_passiveSocket, 5);
	if (rc == -1) {
		ESP_LOGD(LOG_TAG, "listen: %s", strerror(errno));
	}

	unsigned int addrLen = sizeof(serverAddress);
	rc = getsockname(m_passiveSocket, (struct sockaddr*)&serverAddress, &addrLen);
	if (rc == -1) {
		ESP_LOGD(LOG_TAG, "getsockname: %s", strerror(errno));
	}

	std::stringstream ss;
	ss << ((clientAddrInfo.sin_addr.s_addr >> 0) & 0xff) <<
		"," << ((clientAddrInfo.sin_addr.s_addr >> 8) & 0xff) <<
		"," << ((clientAddrInfo.sin_addr.s_addr >> 16) & 0xff) <<
		"," << ((clientAddrInfo.sin_addr.s_addr >> 24) & 0xff) <<
		"," << ((serverAddress.sin_port >> 0) & 0xff) <<
		"," << ((serverAddress.sin_port >> 8) & 0xff);
	std::string retStr = ss.str();

	ESP_LOGD(LOG_TAG, "<< listenPassive: %s", retStr.c_str());
	return retStr;
} // FTPServer#listenPassive


/**
 * Handle the AUTH command.
 */
void FTPServer::onAuth(std::istringstream& ss) {
	std::string param;
	ss >> param;
	ESP_LOGD(LOG_TAG, ">> onAuth: %s", param.c_str());
	sendResponse(FTPServer::RESPONSE_500_COMMAND_UNRECOGNIZED);                // Syntax error, command unrecognized.
	ESP_LOGD(LOG_TAG, "<< onAuth");
} // FTPServer#onAuth


/**
 * Change the current working directory.
 * @param ss A string stream where the first parameter is the directory to change to.
 */
void FTPServer::onCwd(std::istringstream& ss) {
	std::string path;
	ss >> path;
	ESP_LOGD(LOG_TAG, ">> onCwd: path=%s", path.c_str());
	chdir(path.c_str());
	sendResponse(FTPServer::RESPONSE_200_COMMAND_OK);
	ESP_LOGD(LOG_TAG, "<< onCwd");
} // FTPServer#onCwd


/**
 * Process the client transmitted LIST request.
 */
void FTPServer::onList(std::istringstream& ss) {
	std::string directory;
	ss >> directory;
	ESP_LOGD(LOG_TAG, ">> onList: directory=%s", directory.c_str());

	openData();
	sendResponse(FTPServer::RESPONSE_150_ABOUT_TO_OPEN_DATA_CONNECTION); // File status okay; about to open data connection.
	if (m_callbacks != nullptr) {
		std::string dirString = m_callbacks->onDir();
		sendData((uint8_t*) dirString.data(), dirString.length());
	}
	closeData();
	sendResponse(FTPServer::RESPONSE_226_CLOSING_DATA_CONNECTION); // Closing data connection.
	ESP_LOGD(LOG_TAG, "<< onList");
} // FTPServer#onList


void FTPServer::onMkd(std::istringstream& ss) {
	std::string path;
	ss >> path;
	ESP_LOGD(LOG_TAG, ">> onMkd: path=%s", path.c_str());
	sendResponse(FTPServer::RESPONSE_500_COMMAND_UNRECOGNIZED);
	ESP_LOGD(LOG_TAG, "<< onMkd");
} // FTPServer#onMkd


/**
 * Process a NOOP operation.
 */
void FTPServer::onNoop(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onNoop");
	sendResponse(RESPONSE_200_COMMAND_OK); // Command okay.
	ESP_LOGD(LOG_TAG, "<< onNoop");
} // FTPServer#onNoop


/**
 * Process PORT request.  The information provided is encoded in the parameter as
 * h1,h2,h3,h4,p1,p2 where h1,h2,h3,h4 is the IP address we should connect to
 * and p1,p2 is the port number.  The data is MSB
 *
 * Our logic does not form any connection but remembers the ip address and port number
 * to be used for a subsequence data connection.
 *
 * Possible responses:
 * 200
 * 500, 501, 421, 530
 */
void FTPServer::onPort(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onPort");
	char c;
	uint16_t h1, h2, h3, h4, p1, p2;
	ss >> h1 >> c >> h2 >> c >> h3 >> c >> h4 >> c >> p1 >> c >> p2;
	m_dataPort = p1 * 256 + p2;
	ESP_LOGD(LOG_TAG, "%d.%d.%d.%d %d", h1, h2, h3, h4, m_dataPort);
	m_dataIp = h1 << 24 | h2 << 16 | h3 << 8 | h4;
	sendResponse(RESPONSE_200_COMMAND_OK); // Command okay.
	m_isPassive = false;

	ESP_LOGD(LOG_TAG, "<< onPort");
} // FTPServer#onPort


/**
 * Process the PASS command.
 * Possible responses:
 * 230
 * 202
 * 530
 * 500, 501, 503, 421
 * 332
 */
void FTPServer::onPass(std::istringstream& ss) {
	std::string password;
	ss >> password;
	ESP_LOGD(LOG_TAG, ">> onPass: password=%s", password.c_str());

	// If the immediate last command wasn't USER then don't try and process PASS.
	if (m_lastCommand != "USER") {
		sendResponse(RESPONSE_503_BAD_SEQUENCE);
		ESP_LOGD(LOG_TAG, "<< onPass");
		return;
	}

	// Compare the supplied userid and passwords.
	if (m_userid == m_suppliedUserid && password == m_password) {
		sendResponse(RESPONSE_230_USER_LOGGED_IN);
		m_isAuthenticated = true;
	} else {
		sendResponse(RESPONSE_530_NOT_LOGGED_IN);
		closeConnection();
		m_isAuthenticated = false;
	}
	ESP_LOGD(LOG_TAG, "<< onPass");
} // FTPServer#onPass


/**
 * Process the PASV command.
 * Possible responses:
 * 227
 * 500, 501, 502, 421, 530
 */
void FTPServer::onPasv(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onPasv");
	std::string ipInfo = listenPassive();
	std::ostringstream responseTextSS;
	responseTextSS << "Entering Passive Mode (" << ipInfo << ").";
	std::string responseText;
	responseText = responseTextSS.str();
	sendResponse(RESPONSE_227_ENTERING_PASSIVE_MODE, responseText.c_str());
	m_isPassive = true;

	ESP_LOGD(LOG_TAG, "<< onPasv");
} // FTPServer#onPasv


/**
 * Process the PWD command to determine our current working directory.
 * Possible responses:
 * 257
 * 500, 501, 502, 421, 550
 */
void FTPServer::onPWD(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onPWD");
	sendResponse(257, "\"" + getCurrentDirectory() + "\"");
	ESP_LOGD(LOG_TAG, "<< onPWD: %s", getCurrentDirectory().c_str());
} // FTPServer#onPWD


/**
 * Possible responses:
 * 221
 * 500
 */
void FTPServer::onQuit(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onQuit");
	sendResponse(FTPServer::RESPONSE_221_CLOSING_CONTROL_CONNECTION); // Service closing control connection.
	closeConnection();  // Close the connection to the client.
	ESP_LOGD(LOG_TAG, "<< onQuit");
} // FTPServer#onQuit


/**
 * Process a RETR command.  The client sends this command to retrieve the content of a file.
 * The name of the file is the first parameter in the input stream.
 *
 * Possible responses:
 * 125, 150
 *   (110)
 *   226, 250
 *   425, 426, 451
 * 450, 550
 * 500, 501, 421, 530
 * @param ss The parameter stream.
 */
void FTPServer::onRetr(std::istringstream& ss) {
	// We open a data connection back to the client.  We then invoke the callback to indicate that we have
	// started a retrieve operation.  We call the retrieve callback to request the next chunk of data and
	// transmit this down the data connection.  We repeat this until there is no more data to send at which
	// point we close the data connection and we are done.
	ESP_LOGD(LOG_TAG, ">> onRetr");
	std::string fileName;

	ss >> fileName;
	uint8_t data[m_chunkSize];

	if (m_callbacks != nullptr) {
		try {
			m_callbacks->onRetrieveStart(fileName);
		} catch (FTPServer::FileException& e) {
			sendResponse(FTPServer::RESPONSE_550_ACTION_NOT_TAKEN);	  // Requested action not taken.
			ESP_LOGD(LOG_TAG, "<< onRetr: Returned 550 to client.");
			return;
		}
	}

	sendResponse(FTPServer::RESPONSE_150_ABOUT_TO_OPEN_DATA_CONNECTION); // File status okay; about to open data connection.
	openData();
	if (m_callbacks != nullptr) {
		int readSize = m_callbacks->onRetrieveData(data, m_chunkSize);
		while (readSize > 0) {
			sendData(data, readSize);
			readSize = m_callbacks->onRetrieveData(data, m_chunkSize);
		}
	}
	closeData();
	sendResponse(FTPServer::RESPONSE_226_CLOSING_DATA_CONNECTION); // Closing data connection.
	if (m_callbacks != nullptr) {
		m_callbacks->onRetrieveEnd();
	}
	ESP_LOGD(LOG_TAG, "<< onRetr");
} // FTPServer#onRetr


void FTPServer::onRmd(std::istringstream &ss) {
	ESP_LOGD(LOG_TAG, ">> onRmd");
	sendResponse(FTPServer::RESPONSE_500_COMMAND_UNRECOGNIZED);
	ESP_LOGD(LOG_TAG, "<< onRmd");
} // FTPServer#onRmd


/**
 * Called to process a STOR request.  This means that the client wishes to store a file
 * on the server.  The name of the file is found in the parameter.
 */
void FTPServer::onStor(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onStor");
	std::string fileName;
	ss >> fileName;

	receiveFile(fileName);
	ESP_LOGD(LOG_TAG, "<< onStor");
} // FTPServer#onStor


void FTPServer::onSyst(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onSyst");
	sendResponse(215, "UNIX Type: L8");
	ESP_LOGD(LOG_TAG, "<< onSyst");
} // FTPServer#onSyst


/**
 * Process a TYPE request.  The parameter that follows is the type of transfer we wish
 * to process.  Types include:
 * I and A.
 *
 * Possible responses:
 * 200
 * 500, 501, 504, 421, 530
 */
void FTPServer::onType(std::istringstream& ss) {
	ESP_LOGD(LOG_TAG, ">> onType");
	std::string type;
	ss >> type;
	m_isImage = (type.compare("I") == 0);
	sendResponse(FTPServer::RESPONSE_200_COMMAND_OK);   // Command okay.
	ESP_LOGD(LOG_TAG, "<< onType: isImage=%d", m_isImage);
} // FTPServer#onType


/**
 * Process a USER request.  The parameter that follows is the identity of the user.
 *
 * Possible responses:
 * 230
 * 530
 * 500, 501, 421
 * 331, 332
 *
 */
void FTPServer::onUser(std::istringstream& ss) {
	// When we receive a user command, we next want to know if we should ask for a password.  If the m_loginRequired
	// flag is set then we do indeed want a password and will send the response that we wish one.
	std::string userName;
	ss >> userName;
	ESP_LOGD(LOG_TAG, ">> onUser: userName=%s", userName.c_str());
	sendResponse(m_loginRequired ? FTPServer::RESPONSE_331_PASSWORD_REQUIRED : FTPServer::RESPONSE_200_COMMAND_OK);
	m_suppliedUserid = userName;   // Save the username that was supplied.
	ESP_LOGD(LOG_TAG, "<< onUser");
} // FTPServer#onUser


void FTPServer::onXmkd(std::istringstream &ss) {
	ESP_LOGD(LOG_TAG, ">> onXmkd");
	sendResponse(FTPServer::RESPONSE_500_COMMAND_UNRECOGNIZED);
	ESP_LOGD(LOG_TAG, "<< onXmkd");
} // FTPServer#onXmkd


void FTPServer::onXrmd(std::istringstream &ss) {
	ESP_LOGD(LOG_TAG, ">> onXrmd");
	sendResponse(FTPServer::RESPONSE_500_COMMAND_UNRECOGNIZED);
	ESP_LOGD(LOG_TAG, "<< onXrmd");
} // FTPServer#onXrmd


/**
 * Open a data connection with the client.
 * We will use closeData() to close the connection.
 * @return True if the data connection succeeded.
 */
bool FTPServer::openData() {
	if (m_isPassive) {
		// Handle a passive connection ... here we receive a connection from the client from the passive socket.
		struct sockaddr_in clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		m_dataSocket = accept(m_passiveSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
		if (m_dataSocket == -1) {
			ESP_LOGD(LOG_TAG, "FTPServer::openData: accept(): %s", strerror(errno));
			closePassive();
			return false;
		}
		closePassive();
	} else {
		// Handle an active connection ... here we connect to the client.
		m_dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		struct sockaddr_in serverAddress;
		serverAddress.sin_family      = AF_INET;
		serverAddress.sin_addr.s_addr = htonl(m_dataIp);
		serverAddress.sin_port        = htons(m_dataPort);

		int rc = connect(m_dataSocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
		if (rc == -1) {
			ESP_LOGD(LOG_TAG, "FTPServer::openData: connect(): %s", strerror(errno));
			return false;
		}
	}
	return true;
} // FTPServer#openData


/**
 * Process commands received from the client.
 */
void FTPServer::processCommand() {
	sendResponse(FTPServer::RESPONSE_220_SERVICE_READY); // Service ready.
	ESP_LOGD(LOG_TAG, ">> FTPServer::processCommand");
	m_lastCommand = "";
	while (true) {
		std::string line = "";
		char currentChar;
		char lastChar = '\0';
		int rc = recv(m_clientSocket, &currentChar, 1, 0);
		while (rc != -1 && rc != 0) {
			line += currentChar;
			if (lastChar == '\r' && currentChar == '\n') break;
//			printf("%c\n", currentChar);
			lastChar = currentChar;
			rc = recv(m_clientSocket, &currentChar, 1, 0);
		} // End while we are waiting for a line.

		if (rc == 0 || rc == -1) break;		  // If we didn't get a line or an error, then we have finished processing commands.

		std::string command;
		std::istringstream ss(line);
		getline(ss, command, ' ');
		trim(command);

		// We now have a command to process.

		ESP_LOGD(LOG_TAG, "Command: \"%s\"", command.c_str());
		if (command.compare("USER") == 0) {
			onUser(ss);
		}
		else if (command.compare("PASS") == 0) {
			onPass(ss);
		}
		else if (m_loginRequired && !m_isAuthenticated) {
			sendResponse(RESPONSE_530_NOT_LOGGED_IN);
		}
		else if (command.compare("PASV") == 0) {
			onPasv(ss);
		}
		else if (command.compare("SYST") == 0) {
			onSyst(ss);
		}
		else if (command.compare("PORT") == 0) {
			onPort(ss);
		}
		else if (command.compare("LIST") == 0) {
			onList(ss);
		}
		else if (command.compare("TYPE") == 0) {
			onType(ss);
		}
		else if (command.compare("RETR") == 0) {
			onRetr(ss);
		}
		else if (command.compare("QUIT") == 0) {
			onQuit(ss);
		}
		else if (command.compare("AUTH") == 0) {
			onAuth(ss);
		}
		else if (command.compare("STOR") == 0) {
			onStor(ss);
		}
		else if (command.compare("PWD") == 0) {
			onPWD(ss);
		}
		else if (command.compare("MKD") == 0) {
			onMkd(ss);
		}
		else if (command.compare("XMKD") == 0) {
			onXmkd(ss);
		}
		else if (command.compare("RMD") == 0) {
			onRmd(ss);
		}
		else if (command.compare("XRMD") == 0) {
			onXrmd(ss);
		}
		else if (command.compare("CWD") == 0) {
			onCwd(ss);
		}
		else {
			sendResponse(FTPServer::RESPONSE_500_COMMAND_UNRECOGNIZED); // Syntax error, command unrecognized.
		}
		m_lastCommand = command;
	} // End loop processing commands.

	close(m_clientSocket); // We won't be processing any further commands from this client.
	ESP_LOGD(LOG_TAG, "<< FTPServer::processCommand");
} // FTPServer::processCommand


/**
 * Receive a file from the FTP client (STOR).  The name of the file to be created is passed as a
 * parameter.
 */
void FTPServer::receiveFile(std::string fileName) {
	ESP_LOGD(LOG_TAG, ">> receiveFile: %s", fileName.c_str());
	if (m_callbacks != nullptr) {
		try {
			m_callbacks->onStoreStart(fileName);
		} catch(FTPServer::FileException& e) {
			ESP_LOGD(LOG_TAG, "Caught a file exception!");
			sendResponse(FTPServer::RESPONSE_550_ACTION_NOT_TAKEN); // Requested action not taken.
			return;
		}
	}
	openData();
	sendResponse(FTPServer::RESPONSE_150_ABOUT_TO_OPEN_DATA_CONNECTION); // File status okay; about to open data connection.
	uint8_t buf[m_chunkSize];
	uint32_t totalSizeRead = 0;
	while (true) {
		int rc = recv(m_dataSocket, &buf, m_chunkSize, 0);
		if (rc <= 0) break;
		if (m_callbacks != nullptr) {
			m_callbacks->onRetrieveData(buf, rc);
		}
		totalSizeRead += rc;
	}
	sendResponse(FTPServer::RESPONSE_226_CLOSING_DATA_CONNECTION); // Closing data connection.
	closeData();
	if (m_callbacks != nullptr) {
		m_callbacks->onRetrieveEnd();
	}
	ESP_LOGD(LOG_TAG, "<< receiveFile: totalSizeRead=%d", totalSizeRead);
} // FTPServer#receiveFile


/**
 * Send data to the client over the data connection previously opened with a call to openData().
 * @param pData A pointer to the data to send.
 * @param size The number of bytes to send.
 */
void FTPServer::sendData(uint8_t* pData, uint32_t size) {
	ESP_LOGD(LOG_TAG, ">> FTPServer::sendData: size=%d", size);
	int rc = send(m_dataSocket, pData, size, 0);
	if (rc == -1) {
		ESP_LOGD(LOG_TAG, "FTPServer::sendData: send(): %s", strerror(errno));
	}
	ESP_LOGD(LOG_TAG, "<< FTPServer::sendData");
} // FTPServer#sendData


/**
 * Send a response to the client.  A response is composed of two parts.  The first is a code as architected in the
 * FTP specification.  The second is a piece of text.
 */
void FTPServer::sendResponse(int code, std::string text) {
	ESP_LOGD(LOG_TAG, ">> sendResponse: (%d) %s", code, text.c_str());
	std::ostringstream ss;
	ss << code << " " << text << "\r\n";
	int rc = send(m_clientSocket, ss.str().data(), ss.str().length(), 0);
	if (rc == -1) {
		ESP_LOGE(LOG_TAG,"send: %s", strerror(errno));
	}
	ESP_LOGD(LOG_TAG, "<< sendResponse");
} // FTPServer#sendResponse


/**
 * Send a response to the client.  A response is composed of two parts.  The first is a code as architected in the
 * FTP specification.  The second is a piece of text.  In this function, a standard piece of text is used based on
 * the code.
 */
void FTPServer::sendResponse(int code) {
	std::string text = "unknown";

	switch(code) {             // Map the code to a text string.
		case RESPONSE_150_ABOUT_TO_OPEN_DATA_CONNECTION:
			text = "File status okay; about to open data connection.";
			break;
		case RESPONSE_200_COMMAND_OK:
			text = "Command okay.";
			break;
		case RESPONSE_220_SERVICE_READY:
			text = "Service ready.";
			break;
		case RESPONSE_221_CLOSING_CONTROL_CONNECTION:
			text = "Service closing control connection.";
			break;
		case RESPONSE_226_CLOSING_DATA_CONNECTION:
			text = "Closing data connection.";
			break;
		case RESPONSE_230_USER_LOGGED_IN:
			text = "User logged in, proceed.";
			break;
		case RESPONSE_331_PASSWORD_REQUIRED:
			text = "Password required.";
			break;
		case RESPONSE_500_COMMAND_UNRECOGNIZED:
			text = "Syntax error, command unrecognized.";
			break;
		case RESPONSE_502_COMMAND_NOT_IMPLEMENTED:
			text = "Command not implemented.";
			break;
		case RESPONSE_503_BAD_SEQUENCE:
			text = "Bad sequence of commands.";
			break;
		case RESPONSE_530_NOT_LOGGED_IN:
			text = "Not logged in.";
			break;
		case RESPONSE_550_ACTION_NOT_TAKEN:
			text = "Requested action not taken.";
			break;
		default:
			break;
	}
	sendResponse(code, text);   // Send the code AND the text to the FTP client.
} // FTPServer#sendResponse


/**
 * Set the callbacks that are to be invoked to perform work.
 * @param pCallbacks An instance of an FTPCallbacks based class.
 */
void FTPServer::setCallbacks(FTPCallbacks* pCallbacks) {
	m_callbacks = pCallbacks;
} // FTPServer#setCallbacks


void FTPServer::setCredentials(std::string userid, std::string password) {
	ESP_LOGD(LOG_TAG, ">> setCredentials: userid=%s", userid.c_str());
	m_loginRequired = true;
	m_userid        = userid;
	m_password      = password;
	ESP_LOGD(LOG_TAG, "<< setCredentials");
} // FTPServer#setCredentials


/**
 * Set the TCP port we should listen on for FTP client requests.
 */
void FTPServer::setPort(uint16_t port) {
	m_port = port;
} // FTPServer#setPort


/**
 * Start being an FTP Server.
 */
void FTPServer::start() {
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_serverSocket == -1) {
		ESP_LOGD(LOG_TAG, "socket: %s", strerror(errno));
	}

	int enable = 1;
	setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(m_port);
	int rc = bind(m_serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (rc == -1) {
		ESP_LOGD(LOG_TAG, "bind: %s", strerror(errno));
	}
	rc = listen(m_serverSocket, 5);
	if (rc == -1) {
		ESP_LOGD(LOG_TAG, "listen: %s", strerror(errno));
	}
	while (true) {
		waitForFTPClient();
		processCommand();
	}
} // FTPServer#start


/**
 * Wait for a new client to connect.
 */
int FTPServer::waitForFTPClient() {
	ESP_LOGD(LOG_TAG, ">> FTPServer::waitForFTPClient");

	struct sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	m_clientSocket = accept(m_serverSocket, (struct sockaddr*) &clientAddress, &clientAddressLength);

	char ipAddr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddress.sin_addr, ipAddr, sizeof(ipAddr));
	ESP_LOGD(LOG_TAG, "Received connection from %s [%d]", ipAddr, clientAddress.sin_port);

	struct sockaddr_in socketAddressInfo;
	unsigned int socketAddressInfoSize = sizeof(socketAddressInfo);
	getsockname(m_clientSocket, (struct sockaddr*) &socketAddressInfo, &socketAddressInfoSize);

	inet_ntop(AF_INET, &socketAddressInfo.sin_addr, ipAddr, sizeof(ipAddr));
	ESP_LOGD(LOG_TAG, "Connected at %s [%d]", ipAddr, socketAddressInfo.sin_port);
	ESP_LOGD(LOG_TAG, "<< FTPServer::waitForFTPClient: fd=%d\n", m_clientSocket);

	return m_clientSocket;
} // FTPServer::waitForFTPClient

