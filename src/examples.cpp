
#include <fstream>
#include <iostream>

#include "hawkutils/elusivejson.h"

using namespace ElusiveJSON;

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

	JMalloc* malloc = new JMalloc(size);
	JObject* jobj = parseJObject(jsonData, malloc);

	std::cout << jobj->toString(true) << std::endl;

}