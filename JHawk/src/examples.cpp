
#include <fstream>
#include <iostream>

#include "hawkutils/jhawk.h"

int main()
{
	std::string jsonData;
	char rawData[4096]{'\0'};

	std::ifstream file("example.json", std::ios::ate);

	if (!file.good())
	{
		throw std::runtime_error("Can't find example JSON!");
	}

	size_t size = file.tellg();
	file.seekg(0);

	file.read(rawData, size);
	file.close();

	jsonData = std::string(rawData);

	JHawk::JObject* jobj = JHawk::parseJObject(jsonData);

	std::cout << jobj->toString() << std::endl;

}