/*
 * JSON.cpp
 *
 *  Created on: May 23, 2017
 *      Author: kolban
 */


#include <string>
#include <stdlib.h>
#include "JSON.h"

JsonArray JSON::createArray() {
	return JsonArray(cJSON_CreateArray());
}

JsonObject JSON::createObject() {
	return JsonObject(cJSON_CreateObject());
}

void JSON::deleteArray(JsonArray jsonArray) {
	cJSON_Delete(jsonArray.m_node);
}

void JSON::deleteObject(JsonObject jsonObject) {
	cJSON_Delete(jsonObject.m_node);
}

JsonArray JSON::parseArray(std::string text) {
	return JsonArray(cJSON_Parse(text.c_str()));
}

JsonObject JSON::parseObject(std::string text) {
	return JsonObject(cJSON_Parse(text.c_str()));
}

JsonArray::JsonArray(cJSON* node) {
	m_node = node;
}

void JsonArray::addBoolean(bool value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateBool(value));
}


void JsonArray::addDouble(double value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateNumber(value));
}

void JsonArray::addInt(int value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateDouble((double)value, value));
}

void JsonArray::addObject(JsonObject value) {
	cJSON_AddItemToArray(m_node, value.m_node);
}

void JsonArray::addString(std::string value) {
	cJSON_AddItemToArray(m_node, cJSON_CreateString(value.c_str()));
}

bool JsonArray::getBoolean(int item) {
	cJSON *node = cJSON_GetArrayItem(m_node, item);
	if (node->valueint == 0) {
		return false;
	}
	return true;
}

double JsonArray::getDouble(int item) {
	cJSON *node = cJSON_GetArrayItem(m_node, item);
	return node->valuedouble;
}

int JsonArray::getInt(int item) {
	cJSON *node = cJSON_GetArrayItem(m_node, item);
	return node->valueint;
}

JsonObject JsonArray::getObject(int item) {
	cJSON *node = cJSON_GetArrayItem(m_node, item);
	return JsonObject(node);
}

std::string JsonArray::getString(int item) {
	cJSON *node = cJSON_GetArrayItem(m_node, item);
	return std::string(node->valuestring);
}


JsonObject::JsonObject(cJSON* node) {
	m_node = node;
}

bool JsonObject::getBoolean(std::string name) {
	cJSON *node = cJSON_GetObjectItem(m_node, name.c_str());
	if (node->valueint == 0) {
		return false;
	}
	return true;
}

double JsonObject::getDouble(std::string name) {
	cJSON *node = cJSON_GetObjectItem(m_node, name.c_str());
	return node->valuedouble;
}

int JsonObject::getInt(std::string name) {
	cJSON *node = cJSON_GetObjectItem(m_node, name.c_str());
	return node->valueint;
}

JsonObject JsonObject::getObject(std::string name) {
	cJSON *node = cJSON_GetObjectItem(m_node, name.c_str());
	return JsonObject(node);
}

std::string JsonObject::getString(std::string name) {
	cJSON *node = cJSON_GetObjectItem(m_node, name.c_str());
	return std::string(node->valuestring);
}

void JsonObject::setArray(std::string name, JsonArray array) {
	cJSON_AddItemToObject(m_node, name.c_str(), array.m_node);
}

void JsonObject::setBoolean(std::string name, bool value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateBool(value));
}

void JsonObject::setDouble(std::string name, double value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateNumber(value));
}

void JsonObject::setInt(std::string name, int value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateDouble((double)value, value));
}

void JsonObject::setObject(std::string name, JsonObject value) {
	cJSON_AddItemToObject(m_node, name.c_str(), value.m_node);
}

void JsonObject::setString(std::string name, std::string value) {
	cJSON_AddItemToObject(m_node, name.c_str(), cJSON_CreateString(value.c_str()));
}

std::string JsonObject::toString() {
	char *data = cJSON_Print(m_node);
	std::string ret(data);
	free(data);
	return ret;
}
