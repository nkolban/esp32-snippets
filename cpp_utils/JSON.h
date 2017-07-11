/*
 * JSON.h
 *
 *  Created on: May 23, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_JSON_H_
#define COMPONENTS_CPP_UTILS_JSON_H_
#include <cJSON.h>
#include <string>

// Forward declarations
class JsonObject;
class JsonArray;

/**
 * @brief Top level JSON handler.
 */
class JSON {
public:
	static JsonObject createObject();
	static JsonArray createArray();
	static void deleteObject(JsonObject jsonObject);
	static void deleteArray(JsonArray jsonArray);
	static JsonObject parseObject(std::string text);
	static JsonArray parseArray(std::string text);
}; // JSON


/**
 * @brief A JSON array.
 */
class JsonArray {
public:
	JsonArray(cJSON *node);
	int getInt(int item);
	JsonObject getObject(int item);
	std::string getString(int item);
	bool getBoolean(int item);
	double getDouble(int item);
	void addBoolean(bool value);
	void addDouble(double value);
	void addInt(int value);
	void addObject(JsonObject value);
	void addString(std::string value);
	std::string toString();
	std::size_t size();
	/**
	 * @brief The underlying cJSON node.
	 */
	cJSON *m_node;
}; // JsonArray


/**
 * @brief A JSON object.
 */
class JsonObject {
public:
	JsonObject(cJSON *node);
	int getInt(std::string name);
	JsonObject getObject(std::string name);
	std::string getString(std::string name);
	bool getBoolean(std::string name);
	double getDouble(std::string name);
	void setArray(std::string name, JsonArray array);
	void setBoolean(std::string name, bool value);
	void setDouble(std::string name, double value);
	void setInt(std::string name, int value);
	void setObject(std::string name, JsonObject value);
	void setString(std::string name, std::string value);
	std::string toString();
	/**
	 * @brief The underlying cJSON node.
	 */
	cJSON *m_node;
}; // JsonObject


#endif /* COMPONENTS_CPP_UTILS_JSON_H_ */
