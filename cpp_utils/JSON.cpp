/*
 * JSON.cpp
 *
 *  Created on: May 23, 2017
 *      Author: kolban
 */


// See: https://github.com/DaveGamble/cJSON

#include <string>
#include <stdlib.h>
#include "JSON.h"

/**
 * @brief Create an empty JSON array.
 * @return An empty JSON array.
 */
JsonArray JSON::createArray() {
	return JsonArray(cJSON_CreateArray());
} // createArray


/**
 * @brief Create an empty JSON object.
 * @return An empty JSON object.
 */
JsonObject JSON::createObject() {
	return JsonObject(cJSON_CreateObject());
} // createObject


/**
 * @brief Delete a JSON array.
 * @param [in] jsonArray The array to be deleted.
 * @return N/A.
 */
void JSON::deleteArray(JsonArray jsonArray) {
	cJSON_Delete(jsonArray.m_node);
} // deleteArray


/**
 * @brief Delete a JSON object.
 * @param [in] jsonObject The object to be deleted.
 */
void JSON::deleteObject(JsonObject jsonObject) {
	cJSON_Delete(jsonObject.m_node);
} // deleteObject


/**
 * @brief Parse a string that contains a JSON array.
 * @param [in] text The JSON text string.
 * @return A JSON array.
 */
JsonArray JSON::parseArray(std::string text) {
	return JsonArray(cJSON_Parse(text.c_str()));
} // parseArray


/**
 * @brief Parse a string that contains a JSON object.
 * @param [in] text The JSON text string.
 * @return a JSON object.
 */
JsonObject JSON::parseObject(std::string text) {
	return JsonObject(cJSON_Parse(text.c_str()));
} // parseObject


JsonArray::JsonArray(cJSON* node) {
	m_node = node;
}


/**
 * @brief Add a boolean value to the array.
 * @param [in] value The boolean value to add to the array.
 */
void JsonArray::addBoolean(bool value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateBool(value));
} // addBoolean


/**
 * @brief Add a double value to the array.
 * @param [in] value The double value to add to the array.
 */
void JsonArray::addDouble(double value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateNumber(value));
} // addDouble


/**
 * @brief Add an int value to the array.
 * @param [in] value The int value to add to the array.
 */
void JsonArray::addInt(int value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateNumber((double)value));
} // addInt


/**
 * @brief Add an object value to the array.
 * @param [in] value The object value to add to the array.
 */
void JsonArray::addObject(JsonObject value) {
	cJSON_AddItemToArray(m_node, value.m_node);
} // addObject


/**
 * @brief Add a string value to the array.
 * @param [in] value The string value to add to the array.
 */
void JsonArray::addString(std::string value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateString(value.c_str()));
} // addString


/**
 * @brief Get the indexed boolean value from the array.
 * @param [in] item The index of the array to retrieve.
 * @return The boolean value at the given index.
 */
bool JsonArray::getBoolean(int item) {
	cJSON* node = cJSON_GetArrayItem(m_node, item);
	return (node->valueint != 0);
} // getBoolean


/**
 * @brief Get the indexed double value from the array.
 * @param [in] item The index of the array to retrieve.
 * @return The double value at the given index.
 */
double JsonArray::getDouble(int item) {
	cJSON* node = cJSON_GetArrayItem(m_node, item);
	return node->valuedouble;
} // getDouble


/**
 * @brief Get the indexed int value from the array.
 * @param [in] item The index of the array to retrieve.
 * @return The int value at the given index.
 */
int JsonArray::getInt(int item) {
	cJSON* node = cJSON_GetArrayItem(m_node, item);
	return node->valueint;
} // getInt


/**
 * @brief Get the indexed object value from the array.
 * @param [in] item The index of the array to retrieve.
 * @return The object value at the given index.
 */
JsonObject JsonArray::getObject(int item) {
	cJSON* node = cJSON_GetArrayItem(m_node, item);
	return JsonObject(node);
} // getObject


/**
 * @brief Get the indexed object value from the array.
 * @param [in] item The index of the array to retrieve.
 * @return The object value at the given index.
 */
std::string JsonArray::getString(int item) {
	cJSON* node = cJSON_GetArrayItem(m_node, item);
	return std::string(node->valuestring);
} // getString


/**
 * @brief Convert the JSON array to a string.
 * @return A JSON string representation of the array.
 */
std::string JsonArray::toString() {
	char* data = cJSON_Print(m_node);
	std::string ret(data);
	free(data);
	return ret;
} // toString


/**
 * @brief Build an unformatted string representation.
 * @return A string representation.
 */
std::string JsonArray::toStringUnformatted() {
	char* data = cJSON_PrintUnformatted(m_node);
	std::string ret(data);
	free(data);
	return ret;
} // toStringUnformatted


/**
 * @brief Get the number of elements from the array.
 * @return The int value that represents the number of elements.
 */
std::size_t JsonArray::size() {
	return cJSON_GetArraySize(m_node);
} // size

/**
 * @brief Constructor
 */
JsonObject::JsonObject(cJSON* node) {
	m_node = node;
} // JsonObject

JsonArray JsonObject::getArray(std::string name) {
	cJSON* node = cJSON_GetObjectItem(m_node, name.c_str());
	return JsonArray(node);
}


/**
 * @brief Get the named boolean value from the object.
 * @param [in] name The name of the object property.
 * @return The boolean value from the object.
 */
bool JsonObject::getBoolean(std::string name) {
	cJSON* node = cJSON_GetObjectItem(m_node, name.c_str());
	if (node == nullptr) return false;
	return cJSON_IsTrue(node);
} // getBoolean


/**
 * @brief Get the named double value from the object.
 * @param [in] name The name of the object property.
 * @return The double value from the object.
 */
double JsonObject::getDouble(std::string name) {
	cJSON* node = cJSON_GetObjectItem(m_node, name.c_str());
	if (node == nullptr) return 0.0;
	return node->valuedouble;
} // getDouble


/**
 * @brief Get the named int value from the object.
 * @param [in] name The name of the object property.
 * @return The int value from the object.
 */
int JsonObject::getInt(std::string name) {
	cJSON* node = cJSON_GetObjectItem(m_node, name.c_str());
	if (node == nullptr) return 0;
	return node->valueint;
} // getInt


/**
 * @brief Get the named object value from the object.
 * @param [in] name The name of the object property.
 * @return The object value from the object.
 */
JsonObject JsonObject::getObject(std::string name) {
	cJSON* node = cJSON_GetObjectItem(m_node, name.c_str());
	return JsonObject(node);
} // getObject


/**
 * @brief Get the named string value from the object.
 * @param [in] name The name of the object property.
 * @return The string value from the object.  A zero length string is returned when the object is not present.
 */
std::string JsonObject::getString(std::string name) {
	cJSON* node = cJSON_GetObjectItem(m_node, name.c_str());
	if (node == nullptr) return "";
	return std::string(node->valuestring);
} // getString


/**
 * @brief Determine if the object has the specified item.
 * @param [in] name The name of the property to check for presence.
 * @return True if the object contains this property.
 */
bool JsonObject::hasItem(std::string name) {
	return cJSON_GetObjectItem(m_node, name.c_str()) != nullptr;
} // hasItem


/**
 * @brief Determine if this represents a valid JSON node.
 * @return True if this is a valid node and false otherwise.
 */
bool JsonObject::isValid() {
	return m_node != nullptr;
} // isValid

/**
 * @brief Set the named array property.
 * @param [in] name The name of the property to add.
 * @param [in] array The array to add to the object.
 * @return N/A.
 */
void JsonObject::setArray(std::string name, JsonArray array) {
	cJSON_AddItemToObject(m_node, name.c_str(), array.m_node);
} // setArray


/**
 * @brief Set the named boolean property.
 * @param [in] name The name of the property to add.
 * @param [in] value The boolean to add to the object.
 * @return N/A.
 */
void JsonObject::setBoolean(std::string name, bool value) {
	cJSON_AddItemToObject(m_node, name.c_str(), value ? cJSON_CreateTrue() : cJSON_CreateFalse());
} // setBoolean


/**
 * @brief Set the named double property.
 * @param [in] name The name of the property to add.
 * @param [in] value The double to add to the object.
 * @return N/A.
 */
void JsonObject::setDouble(std::string name, double value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateNumber(value));
} // setDouble


/**
 * @brief Set the named int property.
 * @param [in] name The name of the property to add.
 * @param [in] value The int to add to the object.
 * @return N/A.
 */
void JsonObject::setInt(std::string name, int value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateNumber((double) value));
} // setInt


/**
 * @brief Set the named object property.
 * @param [in] name The name of the property to add.
 * @param [in] value The object to add to the object.
 * @return N/A.
 */
void JsonObject::setObject(std::string name, JsonObject value) {
	cJSON_AddItemToObject(m_node, name.c_str(), value.m_node);
} // setObject


/**
 * @brief Set the named string property.
 * @param [in] name The name of the property to add.
 * @param [in] value The string to add to the object.
 * @return N/A.
 */
void JsonObject::setString(std::string name, std::string value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateString(value.c_str()));
} // setString


/**
 * @brief Convert the JSON object to a string.
 * @return A JSON string representation of the object.
 */
std::string JsonObject::toString() {
	char* data = cJSON_Print(m_node);
	std::string ret(data);
	free(data);
	return ret;
} // toString


/**
 * @brief Build an unformatted string representation.
 * @return A string representation.
 */
std::string JsonObject::toStringUnformatted() {
	char* data = cJSON_PrintUnformatted(m_node);
	std::string ret(data);
	free(data);
	return ret;
} // toStringUnformatted
