/*
 * FTPServer.h
 *
 *  Created on: May 6, 2018
 *      Author: kolban
 */

#ifndef NETWORKING_FTPSERVER_FTPSERVER_H_
#define NETWORKING_FTPSERVER_FTPSERVER_H_
#include <stdint.h>
#include <sstream>
#include <fstream>
#include <string>
#include <exception>


class FTPCallbacks {
public:
	virtual void        onStoreStart(std::string fileName);
	virtual size_t      onStoreData(uint8_t* data, size_t size);
	virtual void        onStoreEnd();
	virtual void        onRetrieveStart(std::string fileName);
	virtual size_t      onRetrieveData(uint8_t *data, size_t size);
	virtual void        onRetrieveEnd();
	virtual std::string onDir();
	virtual ~FTPCallbacks();

};

/**
 * An implementation of FTPCallbacks that uses Posix File I/O to perform file access.
 */
class FTPFileCallbacks : public FTPCallbacks {
public:
	void        onStoreStart(std::string fileName) override;			// Called for a STOR request.
	size_t      onStoreData(uint8_t* data, size_t size) override;	    // Called when a chunk of STOR data becomes available.
	void        onStoreEnd() override;								    // Called at the end of a STOR request.
	void        onRetrieveStart(std::string fileName) override;		    // Called at the start of a RETR request.
	size_t      onRetrieveData(uint8_t* data, size_t size) override;	// Called to retrieve a chunk of RETR data.
	void        onRetrieveEnd() override;							    // Called when we have retrieved all the data.
	std::string onDir() override;									    // Called to retrieve all the directory entries.

private:
	std::ofstream m_storeFile;	  // File used to store data from the client.
	std::ifstream m_retrieveFile;   // File used to retrieve data for the client.
	uint32_t	  m_byteCount;	  // Count of bytes sent over wire.

};


class FTPServer {
public:
	FTPServer();
	virtual ~FTPServer();
	void setCredentials(std::string userid, std::string password);
	void start();
	void setPort(uint16_t port);
	void setCallbacks(FTPCallbacks* pFTPCallbacks);
	static std::string getCurrentDirectory();
	class FileException: public std::exception {
	};

	// Response codes.
	static const int RESPONSE_150_ABOUT_TO_OPEN_DATA_CONNECTION = 150;
	static const int RESPONSE_200_COMMAND_OK					= 200;
	static const int RESPONSE_202_COMMAND_NOT_IMPLEMENTED	   = 202;
	static const int RESPONSE_212_DIRECTORY_STATUS			  = 212;
	static const int RESPONSE_213_FILE_STATUS				   = 213;
	static const int RESPONSE_214_HELP_MESSAGE				  = 214;
	static const int RESPONSE_220_SERVICE_READY				 = 220;
	static const int RESPONSE_221_CLOSING_CONTROL_CONNECTION	= 221;
	static const int RESPONSE_230_USER_LOGGED_IN				= 230;
	static const int RESPONSE_226_CLOSING_DATA_CONNECTION	   = 226;
	static const int RESPONSE_227_ENTERING_PASSIVE_MODE		 = 227;
	static const int RESPONSE_331_PASSWORD_REQUIRED			 = 331;
	static const int RESPONSE_332_NEED_ACCOUNT				  = 332;
	static const int RESPONSE_500_COMMAND_UNRECOGNIZED		  = 500;
	static const int RESPONSE_502_COMMAND_NOT_IMPLEMENTED	   = 502;
	static const int RESPONSE_503_BAD_SEQUENCE				  = 503;
	static const int RESPONSE_530_NOT_LOGGED_IN				 = 530;
	static const int RESPONSE_550_ACTION_NOT_TAKEN			  = 550;
	static const int RESPONSE_553_FILE_NAME_NOT_ALLOWED		 = 553;

private:
	int         m_serverSocket;   // The socket the FTP server is listening on.
	int         m_clientSocket;   // The current client socket.
	int         m_dataSocket;     // The data socket.
	int         m_passiveSocket;  // The socket on which the server is listening for passive FTP connections.
	uint16_t    m_port;           // The port the FTP server will use.
	uint16_t    m_dataPort;       // The port for data connections.
	uint32_t    m_dataIp;         // The ip address for data connections.
	bool        m_isPassive;      // Are we in passive mode?  If not, then we are in active mode.
	bool        m_isImage;        // Are we in image mode?
	size_t      m_chunkSize;      // The maximum chunk size.
	std::string m_userid;         // The required userid.
	std::string m_password;       // The required password.
	std::string m_suppliedUserid; // The userid supplied from the USER command.
	bool        m_loginRequired;  // Do we required a login?
	bool        m_isAuthenticated;  // Have we authenticated?
	std::string m_lastCommand;    // The last command that was processed.

	FTPCallbacks*    m_callbacks;  // The callbacks for processing.

	void closeConnection();
	void closeData();
	void closePassive();
	void onAuth(std::istringstream& ss);
	void onCwd(std::istringstream& ss);
	void onList(std::istringstream& ss);
	void onMkd(std::istringstream& ss);
	void onNoop(std::istringstream& ss);
	void onPass(std::istringstream& ss);
	void onPasv(std::istringstream& ss);
	void onPort(std::istringstream& ss);
	void onPWD(std::istringstream& ss);
	void onQuit(std::istringstream& ss);
	void onRetr(std::istringstream& ss);
	void onRmd(std::istringstream& ss);
	void onStor(std::istringstream& ss);
	void onSyst(std::istringstream& ss);
	void onType(std::istringstream& ss);
	void onUser(std::istringstream& ss);
	void onXmkd(std::istringstream& ss);
	void onXrmd(std::istringstream& ss);

	bool openData();

	void receiveFile(std::string fileName);
	void sendResponse(int code);
	void sendResponse(int code, std::string text);
	void sendData(uint8_t* pData, uint32_t size);
	std::string listenPassive();
	int waitForFTPClient();
	void processCommand();

};

#endif /* NETWORKING_FTPSERVER_FTPSERVER_H_ */
