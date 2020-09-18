
#pragma once

#include <cstdio>
#include <limits>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/*
TODO:

Add support for wide chars, wide strings, etc.
*/

namespace ElusiveJSON
{
	//Core API

	enum class JType
	{
		INT, FLOAT, BOOL, STRING, ARRAY, OBJECT
	};

	class JValue
	{
	public:
		//Type getter
		virtual const JType type() = 0;
		//Primitive/basic type getters
		virtual bool boolValue() { return false; };
		virtual int intValue() { return 0; };
		virtual float floatValue() { return 0.0f; };
		virtual std::string charValue() { return ""; };
		//Object getters/setters
		virtual bool hasValue(std::string key) { return false; };
		virtual JValue* getValue(std::string key) { return nullptr; };
		virtual void setValue(std::string key, JValue* value) {};
		//Array gettes/setters
		virtual size_t arrayLength() { return 0; };
		virtual JValue* getValue(uint32_t index) { return nullptr; };
		virtual void setValue(uint32_t index, JValue* value) {};
		//Method to print back to JSON
		virtual std::string toString(bool pretty = false, int scope = 1) = 0;

	};

	class JBool : public JValue
	{
	private:
		bool value;
	public:
		JBool(bool b) : value(b) {}

		const JType type() { return JType::BOOL; }

		bool boolValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 1)
		{
			return value ? "true" : "false";
		}

	};

	class JInt : public JValue
	{
	private:
		int value;
	public:
		JInt(int i) : value(i) {}

		const JType type() { return JType::INT; }

		int intValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 1)
		{
			return (std::stringstream() << value).str();
		}
		
	};

	class JFloat : public JValue
	{
	private:
		float value;
	public:
		JFloat(float f) : value(f) {}

		const JType type() { return JType::FLOAT; }

		float floatValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 1)
		{
			return (std::stringstream() << value).str();
		}

	};

	class JString : public JValue
	{
	private:
		const char* string;
		const size_t length;
	public:
		JString(const char* text, size_t len) : string(text), length(len){}

		const JType type() { return JType::STRING; }

		std::string charValue()
		{
			return std::string(string, length);
		}

		std::string toString(bool pretty = false, int scope = 1)
		{
			return (std::stringstream() << '\"' << charValue() << '\"').str();
		}

	};

	class JObject : public JValue
	{
	private:
		std::unordered_map<std::string, JValue*> map = {};
	public:
		JObject() {}

		const JType type() { return JType::OBJECT; }

		bool hasValue(std::string key)
		{
			return map.find(key) != map.end();
		}

		JValue* getValue(std::string key)
		{
			auto val = map.find(key);
			return val != map.end() ? val->second : nullptr;
		}

		void setValue(std::string key, JValue* value)
		{
			map.insert_or_assign(key, value);
		}

		std::string toString(bool pretty = false, int scope = 1)
		{
			std::stringstream ss;

			ss << '{';

			if (pretty)
			{
				ss << '\n';

				for (int s = 0; s < scope; ++s)
				{
					ss << '\t';

				}

			}

			int i = 0;

			for (auto pair : map)
			{
				if (i > 0)
				{
					ss << ',';

					if (pretty)
					{
						ss << '\n';

						for (int s = 0; s < scope; ++s)
						{
							ss << '\t';

						}

					}

				}

				ss << '\"';
				ss << pair.first;
				ss << "\": ";

				//DO NOT UNWRAP THIS TERNARY OPERATOR!!
				ss << (pair.second ? pair.second->toString(pretty, scope + 1) : "null");

				++i;

			}

			if (pretty)
			{
				ss << '\n';

				for (int s = 0; s < scope - 1; ++s)
				{
					ss << '\t';

				}

			}

			ss << '}';

			return ss.str();
		}

	};

	class JArray : public JValue
	{
	private:
		JValue** array;
		size_t length;
	public:
		JArray(JValue** vs, size_t len) : array(vs), length(len){}

		const JType type() { return JType::ARRAY; }

		size_t arrayLength()
		{
			return length;
		}

		JValue* getValue(uint32_t index)
		{
			return array[index];
		}

		void setValue(uint32_t index, JValue* value)
		{
			array[index] = value;
		}

		std::string toString(bool pretty = false, int scope = 1)
		{
			std::stringstream ss;

			ss << '[';

			for (size_t i = 0; i < length; ++i)
			{
				if (i > 0)
				{
					ss << ',';
				}

				JValue* val = array[i];

				ss << (val ? val->toString(pretty, scope + 1) : "null");

			}

			ss << ']';

			return ss.str();
		}

	};

	//Custom linear allocator
	class JMalloc
	{
	private:
		char* data;
		const size_t length;
		size_t current = 0;
		JMalloc* next = nullptr;

	public:
		JMalloc(size_t expected) : length(expected), data(new char[expected] {0}) {}

		JMalloc() : JMalloc(4096) {}

		JMalloc(JMalloc* old) : JMalloc(old->memTotal())
		{
			old->copy(data, 0);
		}

		~JMalloc()
		{
			delete[] data;
			delete next;
		}

		size_t memTotal()
		{
			return length + (next ? next->memTotal() : 0);
		}

		size_t memUsed()
		{
			return current + (next ? next->memUsed() : 0);
		}

		void copy(char* to, size_t offset)
		{
			std::memcpy(to + offset, data, length);

			if (next)
			{
				next->copy(to, offset + length);
			}
		}

		void* allocate(size_t alloc, size_t align = 1)
		{
			if (!alloc || !align)
			{
				return nullptr;
			}

			auto nextMem = ((current + align - 1) / align * align) + alloc;

			if (nextMem > length)
			{
				if (!next)
				{
					next = new JMalloc((alloc + length - 1) / length * length);
				}

				return next->allocate(alloc, align);
			}

			void* mem = (void*)(&data[nextMem - alloc]);
			current = nextMem;

			return mem;
		}

		template<typename T>
		T* allocate()
		{
			return (T*)allocate(sizeof(T), alignof(T));
		}

		void clear(bool secure = true)
		{
			if (secure)
			{
				std::memset(data, 0, current);
			}

			current = 0;

			if (next)
			{
				next->clear(secure);
			}

		}

		JBool* allocBool(bool v) { return new(allocate<JBool>()) JBool(v); }
		JInt* allocInt(int v) { return new(allocate<JInt>()) JInt(v); }
		JFloat* allocFloat(float v) { return new(allocate<JFloat>()) JFloat(v); }
		JArray* allocArray(JValue** arr, size_t length) { return new(allocate<JArray>()) JArray(arr, length); }
		JObject* allocObject() { return new(allocate<JObject>()) JObject(); }

		JString* allocString(std::string* str)
		{
			char* strMem = (char*)allocate(str->length());
			std::memcpy(strMem, str->c_str(), str->length());

			return new(allocate<JString>()) JString(strMem, str->length());
		}

	};

	bool isInt(char c)
	{
		return (c >= '0' && c <= '9');
	}

	bool isHexInt(char c)
	{
		return isInt(c) || (c >= 'A' && c <= 'F');
	}

	bool isASCIILetter(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	class JParser
	{
	private:
		JMalloc* const malloc;
		std::string* text;
		uint64_t current = 0;
		uint32_t line = 1;
		uint32_t lineChar = 1;
		bool useJSON5 = false;

	public:
		JParser(JMalloc* mem, std::string* jsonText) : malloc(mem), text(jsonText){}
		
		JParser* enableJSON5()
		{
			useJSON5 = true;
			return this;
		}

	private:
		void next(uint32_t incr = 1)
		{
			current += incr;
			lineChar += incr;
		}

		void newline()
		{
			++current;
			++line;
			lineChar = 1;
		}

		void skipWhitespace()
		{
			while (true)
			{
				if (useJSON5 && text->at(current) == '/')
				{
					//Comment skipping
					if (text->at(current + 1) == '/')
					{
						next(2);
						while (text->at(current) != '\n') { next(); }
						newline();

					}
					else if (text->at(current + 1) == '*')
					{
						next(2);

						while (text->at(current) != '*' && text->at(current + 1) != '/')
						{
							if (text->at(current) == '\n')
								newline();
							else
								next();
						}

						next(2);

					}
					else
					{
						//FIXME
						throw std::exception("Invalid syntax");
					}

				}

				switch (text->at(current))
				{
					case '\n': newline(); continue;
					case ' ':
					case '\r':
					case '\t': next(); continue;
					default: break;
				}

				break;
			}

		}

		JBool* parseJBool()
		{
			if (text->substr(current, 4) == "true")
			{
				next(4);
				return malloc->allocBool(true);
			}
			else if (text->substr(current, 5) == "false")
			{
				next(5);
				return malloc->allocBool(false);
			}

			return nullptr;
		}

		JValue* parseJIntOrFloat()
		{
			std::stringstream ss;
			bool isHex = false;
			bool isFloat = false;
			bool hasExponent = false;
			int consumed = 0;//For sanity checking against non-JSON 5 ints
			int sign = 1;

			if (text->at(current) == '-')
			{
				next();
				sign = -1;
			}

			if (useJSON5)
			{
				auto sub = text->substr(current, 3);

				//TODO use proper ∞ sign
				if (sub == "Inf")
				{
					next();
					return malloc->allocFloat(std::numeric_limits<float>::infinity() * sign);
				}

				if (sub == "NaN")
				{
					next();
					return malloc->allocFloat(NAN * sign);
				}

				if (text->at(current) == '0' && text->at(current + 1) == 'x')
				{
					isHex = true;
					ss << std::hex;
					ss << text->at(current) << text->at(current + 1);
					next(2);
					consumed += 2;

				}

			}

			while (true)
			{
				char c = text->at(current);

				if (isInt(c) || (isHex && isHexInt(c)))
				{
					ss << c;
					next();
					++consumed;

				}
				else if (c == '.')
				{
					if (isHex || isFloat || hasExponent)
					{
						char buf[256];
						sprintf_s(buf, 256, "Invalid char found in int at %u:%u: \'%c\'", line, lineChar, c);
						throw std::exception(buf);
					}

					if (!useJSON5 && (consumed == 0 || !isInt(text->at(current + 1))))
					{
						char buf[256];
						sprintf_s(buf, 256, "Invalid char found in int at line %u:%u: \'%c\'", line, lineChar, c);
						throw std::exception(buf);
					}

					isFloat = true;
					ss << c;
					next();
					++consumed;

				}
				else if (c == 'e' || c == 'E')
				{
					if (hasExponent)
					{
						char buf[256];
						sprintf_s(buf, 256, "Duplicate exponent in int at line %u:%u", line, lineChar);
						throw std::exception(buf);
					}

					hasExponent = true;
					ss << c;
					next();
					++consumed;

					char expSign = text->at(current);

					if (expSign != '-' && expSign != '+' && !isInt(c))//Apparently signage is optional
					{
						char buf[256];
						sprintf_s(buf, 256, "Invalid exponent signage in int at line %u:%u: \'%c\'", line, lineChar, c);
						throw std::exception(buf);
					}

					ss << c;
					next();
					++consumed;

				}
				else
				{
					//Don't need to have any consequences for an invalid int (i.e. 400j) because after this, it will probably check for a comma. It won't find the comma, and complain.
					break;
				}

			}

			if (isFloat)
			{
				float f;
				ss >> f;
				return malloc->allocFloat(f * sign);
			}

			int i;
			ss >> i;

			return malloc->allocInt(i * sign);
		}

		char getEscapedChar(char c)
		{
			bool valid = true;

			if (c == '\\')
			{
				//TODO Add support for full UTF-8 string literals (means going over to widechar_t)
				//When implemented, this will still return just one character
				//Right now the current workaround is to return the backslash, then it'll interpret the next 5 chars
				//as actual characters. Little messy, but eh.
				if (text->at(current + 1) == 'u')
				{
					for (int i = 2; i <= 5; ++i)
					{
						if (!isHexInt(text->at(current + i)))
						{
							char buf[256];
							sprintf_s(buf, 256, "Malformed UTF-8 character literal at line %u:%u", line, lineChar);
							throw std::exception(buf);
						}

					}

					return c;
				}

				next();

				bool valid = true;
				char nxt = text->at(current);

				switch (nxt)
				{
					case '\"': c = '\"'; break;
					case '\'': c = '\''; break;
					case '\n':
					case 'n': c = '\n'; break;
					case 't': c = '\t'; break;
					case '/': c = '/'; break;
					case '\\': c = '\\'; break;
					case 'b': c = '\b'; break;
					case 'r': c = '\r'; break;
					case 'f': c = '\f'; break;
					default: valid = false;
				}

				if (!valid)
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalid value at line %u:%u: \'%c\'", line, lineChar, c);
					throw std::exception(buf);
				}

			}
			
			return c;
		}

		void parseString(std::string& str)
		{
			char delim = text->at(current);

			if (delim != '\"' && (!useJSON5 || delim != '\''))
			{
				char buf[256];
				sprintf_s(buf, 256, "Invalid string literal at line %u:%u: \'%c\'", line, lineChar, delim);
				throw std::exception(buf);
			}

			next();

			std::stringstream ss;

			char c;

			while ((c = text->at(current)) != delim)
			{
				ss << getEscapedChar(c);

				if (c == '\n')
				{
					if (!useJSON5)
					{
						throw std::exception("Newlines not allowed in strings");
					}

					newline();

				}
				else
				{
					next();
				}

			}

			next();

			str = ss.str();

		}

		void parseUnquotedKey(std::string str)
		{
			uint64_t start = current;
			uint64_t count = 0;

			while (isASCIILetter(text->at(current)))
			{
				next();
				++count;
			}

			if (count == 0)
			{
				char buf[256];
				sprintf_s(buf, 256, "Invalid key value at line %u:%u: \'%c\'", line, lineChar, text->at(current));
				throw std::exception(buf);
			}

			str = text->substr(start, count);

		}

		void parseKey(std::string& str)
		{
			if (useJSON5 && isASCIILetter(text->at(current)))
			{
				parseUnquotedKey(str);
			}
			else
			{
				parseString(str);
			}

		}

		JArray* parseJArray()
		{
			std::vector<JValue*> vals;
			bool expectNextObj = true;

			while (true)
			{
				skipWhitespace();

				if (text->at(current) == ']')
				{
					if (!useJSON5 && expectNextObj && !vals.empty())
					{
						char buf[256];
						sprintf_s(buf, 256, "Trailing comma found at line %u:%u", line, lineChar);
						throw std::exception(buf);
					}

					next();
					break;
				}
				
				if (!expectNextObj)
				{
					char buf[256];
					sprintf_s(buf, 256, "Malformed array at line %u:%u", line, lineChar);
					throw std::exception(buf);
				}

				vals.push_back(parseJValue());

				skipWhitespace();

				if (text->at(current) == ',')
				{
					expectNextObj = true;
					next();
				}
				else
				{
					expectNextObj = false;

				}
				
			}

			JValue** array = (JValue**)malloc->allocate(vals.size() * sizeof(void*), sizeof(void*));

			for (uint32_t i = 0; i < vals.size(); ++i)
			{
				array[i] = (JValue*)vals.at(i);

			}

			return malloc->allocArray(array, vals.size());
		}

	public:
		JValue* parseJValue();

		JObject* parseJObject()
		{
			if (text->at(current) != '{')
			{
				char buf[256];
				sprintf_s(buf, 256, "Invalid JSON object at line %u:%u", line, lineChar);
				throw std::exception(buf);
			}

			next();

			JObject* ret = malloc->allocObject();
			bool expectNextPair = true;
			bool trailingComma = false;
			
			while (true)
			{
				skipWhitespace();

				if (text->at(current) == '}')
				{
					if (!useJSON5 && trailingComma)
					{
						char buf[256];
						sprintf_s(buf, 256, "Trailing comma found before line %u:%u", line, lineChar);
						throw std::exception(buf);
					}

					next();
					break;
				}
				
				if (!expectNextPair)
				{
					char buf[256];
					sprintf_s(buf, 256, "Malformed object at line %u:%u", line, lineChar);
					throw std::exception(buf);
				}

				trailingComma = false;

				std::string key;
				parseKey(key);

				skipWhitespace();

				if (text->at(current) != ':')
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalid value at %u:%u: \'%c\', was expecting \':\'", line, lineChar, text->at(current));
					throw std::exception(buf);
				}

				next();

				skipWhitespace();

				JValue* val = parseJValue();
				ret->setValue(key, val);

				skipWhitespace();

				if (text->at(current) == ',')
				{
					expectNextPair = true;
					trailingComma = true;
					next();

				}
				else
				{
					expectNextPair = false;
					trailingComma = false;

				}

			}

			return ret;
		}

	};

	JValue* JParser::parseJValue()
	{
		char startC = text->at(current);

		if (isInt(startC) || startC == '-' || (useJSON5 && (startC == '.' || startC == '+')))
		{
			return parseJIntOrFloat();
		}

		if (startC == '{')
		{
			return parseJObject();
		}

		if (startC == '[')
		{
			next();
			return parseJArray();
		}

		if (startC == 't' || startC == 'f')
		{
			JBool* bVal = parseJBool();
			if (bVal) return bVal;
		}

		//TODO figure out what to do with this, this looks terrible
		if (startC == 'n' && text->substr(current, 4) == "null")
		{
			next(4);
			return nullptr;
		}

		std::string parsedStr;
		parseString(parsedStr);

		return malloc->allocString(&parsedStr);
	}

}