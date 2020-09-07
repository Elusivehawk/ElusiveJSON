
#pragma once

#include <cstdio>
#include <limits>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ElusiveJSON
{
	constexpr bool JSON5_ENABLE = true;
	constexpr bool JSON5_DISABLE = false;

	class NoImplError : public std::logic_error
	{
	public:
		NoImplError() : std::logic_error("Method or function not yet implemented") {}
	};

#define NOT_IMPL_YET {throw new NoImplError();}

	//FOR INTERNAL USE ONLY
	//PLEASE DO NOT USE
	struct JReadData
	{
		uint64_t current = 0;
		uint32_t line = 0;
		uint32_t lineChar = 0;

		void next(int incr = 1)
		{
			current += incr;
			lineChar += incr;
		}

		void newline()
		{
			++current;
			++line;
			lineChar = 0;
		}

	};

	class JValue
	{
	public:
		virtual bool boolValue() NOT_IMPL_YET;
		virtual int intValue() NOT_IMPL_YET;
		virtual float floatValue() NOT_IMPL_YET;
		virtual std::string strValue() NOT_IMPL_YET;
		virtual JValue* getValue(std::string name) NOT_IMPL_YET;
		virtual void setValue(std::string name, JValue* value) NOT_IMPL_YET;
		virtual size_t length() NOT_IMPL_YET;
		virtual JValue* getValue(uint32_t index) NOT_IMPL_YET;
		virtual void setValue(uint32_t index, JValue* value) NOT_IMPL_YET;
		virtual std::string toString(bool pretty = false, int scope = 0) NOT_IMPL_YET;

	};

	class JBool : public JValue
	{
	private:
		bool value;
	public:
		JBool(bool b) : value(b) {}

		bool boolValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 0)
		{
			std::stringstream ss;

			ss << value;

			return ss.str();
		}

	};

	class JInt : public JValue
	{
	private:
		int value;
	public:
		JInt(int i) : value(i) {}

		int intValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 0)
		{
			std::stringstream ss;

			ss << value;

			return ss.str();
		}
		
	};

	class JFloat : public JValue
	{
	private:
		float value;
	public:
		JFloat(float f) : value(f) {}

		float floatValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 0)
		{
			std::stringstream ss;

			ss << value;

			return ss.str();
		}

	};

	class JString : public JValue
	{
	private:
		const char* string;
		const size_t length;
	public:
		JString(const char* str, size_t len) : string(str), length(len){}

		std::string strValue()
		{
			return std::string(string, length);
		}

		std::string toString(bool pretty = false, int scope = 0)
		{
			std::stringstream ss;
			
			ss << '\"';
			ss << strValue();
			ss << '\"';

			return ss.str();
		}

	};

	class JObject : public JValue
	{
	private:
		std::unordered_map<std::string, JValue*> values = {};
	public:
		JObject(){}

		JValue* getValue(std::string name)
		{
			return values.at(name);
		}

		void setValue(std::string name, JValue* value)
		{
			values.insert_or_assign(name, value);
		}

		std::string toString(bool pretty = false, int scope = 0)
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

			for (auto pair : values)
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

				if (pair.second)
				{
					ss << pair.second->toString(pretty, scope + 1);

				}
				else
				{
					ss << "null";

				}
				
				++i;

			}

			if (pretty)
			{
				ss << '\n';

				for (int s = 0; s < scope; ++s)
				{
					ss << '\t';

				}

			}

			ss << "}";

			return ss.str();
		}

	};

	class JArray : public JValue
	{
	private:
		JValue** values;
		size_t length;
	public:
		JArray(JValue** vs, size_t len) : values(vs), length(len){}

		JValue* getValue(uint32_t index)
		{
			return values[index];
		}

		void setValue(uint32_t index, JValue* value)
		{
			values[index] = value;
		}

		std::string toString(bool pretty = false, int scope = 0)
		{
			std::stringstream ss;

			ss << '[';

			for (size_t i = 0; i < length; ++i)
			{
				if (i > 0)
				{
					ss << ',';
				}

				JValue* val = values[i];

				if (val)
				{
					ss << val->toString();

				}
				else
				{
					ss << "null";

				}

			}

			ss << ']';

			return ss.str();
		}

	};

	class JMalloc
	{
	private:
		char* data;
		const size_t length;
		size_t current = 0;
		JMalloc* next = nullptr;

	public:
		JMalloc(size_t expected) : length(expected), data(new char[expected]) {}

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
			return current;
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

		void clear(bool secure = true)
		{
			if (secure)
			{
				std::memset(data, 0, memUsed());
			}

			current = 0;

			if (next)
			{
				next->clear(secure);
			}

		}

		JBool* allocBool(bool v) { return new(allocate(1)) JBool(v); }
		JInt* allocInt(int v) { return new(allocate(4, 4)) JInt(v); }
		JFloat* allocFloat(float v) { return new(allocate(4, 4)) JFloat(v); }
		JString* allocString(const char* str, size_t length) { return new(allocate(sizeof(JString), 8)) JString(str, length); }
		JArray* allocArray(JValue** arr, size_t length) { return new(allocate(sizeof(JArray), 8)) JArray(arr, length); }
		JObject* allocObject() { return new(allocate(sizeof(JObject), 8)) JObject(); }

	};

	inline bool isInt(char c)
	{
		return (c >= '0' && c <= '9');
	}

	inline bool isHexInt(char c)
	{
		return isInt(c) || (c >= 'A' && c <= 'F');
	}

	bool isIntStart(char c, bool enableJSON5)
	{
		return isInt(c) || c == '-' || (enableJSON5 && ( c == '.' || c == '+'));
	}

	bool isASCIILetter(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	//Core API functions

	void skipWhitespace(std::string str, JReadData& read, bool enableJSON5)
	{
		while (true)
		{
			if (enableJSON5 && str[read.current] == '/')
			{
				//Comment skipping
				if (str[read.current + 1] == '/')
				{
					read.next(2);
					while (str[read.current] != '\n') { read.next(); }
					read.newline();

				}
				else if (str[read.current + 1] == '*')
				{
					read.next(2);

					while (str[read.current] != '*' && str[read.current + 1] != '/')
					{
						if (str[read.current] == '\n')
							read.newline();
						else
							read.next();
					}

					read.next(2);

				}
				else
				{
					//FIXME
					throw std::exception("Invalud syntax");
				}

			}

			switch (str[read.current])
			{
				case '\n': read.newline(); continue;
				case ' ':
				case '\r':
				case '\t': read.next(); continue;
				default: break;
			}

			break;
		}

	}

	JValue* parseJValue(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5);

	JBool* parseJBool(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		if (str.substr(read.current, 4) == "true")
		{
			read.next(4);
			return malloc->allocBool(true);
		}
		else if (str.substr(read.current, 5) == "false")
		{
			read.next(5);
			return malloc->allocBool(false);
		}

		return nullptr;
	}

	JValue* parseJIntOrFloat(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		std::stringstream ss;
		bool isHex = false;
		bool isFloat = false;
		bool hasExponent = false;
		int consumed = 0;//For sanity checking against non-JSON 5 ints
		int sign = 1;

		if (str[read.current] == '-')
		{
			read.next();
			sign = -1;
		}

		if (enableJSON5)
		{
			if (str[read.current] == '∞')
			{
				read.next();
				return malloc->allocFloat(std::numeric_limits<float>::infinity() * sign);
			}

			if (str.substr(read.current, 3) == "NaN")
			{
				read.next();
				return malloc->allocFloat(NAN * sign);
			}

			if (str[read.current] == '0' && str[read.current + 1] == 'x')
			{
				isHex = true;
				ss << std::hex;
				ss << str[read.current] << str[read.current + 1];
				read.next(2);
				consumed += 2;

			}

		}

		while (true)
		{
			char c = str[read.current];

			if (isInt(c) || (isHex && isHexInt(c)) /*Doesn't need to be checked against ELUSIVEJSON_DISABLE_JSON5 because of the isHex flag*/)
			{
				ss << c;
				read.next();
				++consumed;
				continue;
			}
			else if (c == '.')
			{
				if (isHex || isFloat || hasExponent)
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud char found in int at %u:%u: \'%c\'", read.line, read.lineChar, c);
					throw std::exception(buf);
				}

				if (!enableJSON5 && (consumed == 0 || !isInt(str[read.current + 1])))
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud char found in int at line %u:%u: \'%c\'", read.line, read.lineChar, c);
					throw std::exception(buf);
				}

				isFloat = true;
				ss << c;
				read.next();
				++consumed;
				continue;
			}
			else if (c == 'e' || c == 'E')
			{
				hasExponent = true;
				ss << c;
				read.next();
				++consumed;

				char expSign = str[read.current];

				if (expSign != '-' && expSign != '+' && !isInt(c))//Apparently signage is optional
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud exponent signage in int at line %u:%u: \'%c\'", read.line, read.lineChar, c);
					throw std::exception(buf);
				}

				ss << c;
				read.next();
				++consumed;
				continue;
			}

			//Don't need to have any consequences for an invalid int (i.e. 400j) because after this, it will probably check for a comma. It won't find the comma, and complain.
			break;
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

	std::string parseStringValue(const std::string& str, JReadData& read, JMalloc*& malloc, char delim, bool enableJSON5)
	{
		std::stringstream ss;

		while (true)
		{
			char c = str[read.current];
			read.next();

			if (c == delim)
			{
				break;
			}

			if (c == '\\')
			{
				bool valid = true;
				char nxt = str[read.current];/*now points to the next char*/

				switch (nxt)
				{
					case '\"': c = '\"'; read.next(); break;// it could confuse an escaped quote for an end quote
					case 'n': c = '\n'; break;
					case 'u': {
						for (int i = 1; i <= 4; ++i)
						{
							if (!isHexInt(str[read.current + i]))
							{
								char buf[256];
								sprintf_s(buf, 256, "Malformed UTF-8 string literal at line %u:%u", read.line, read.lineChar);
								throw std::exception(buf);
							}
						}
					} break;
					case 't': c = '\t'; break;
					case '/': c = '/'; break;
					case '\\': c = '\\'; break;
					case 'b': c = '\b'; break;
					case 'r': c = '\r'; break;
					case 'f': c = '\f'; break;
					default: valid = false;
				}

				if (enableJSON5 && (nxt == '\'' || nxt == '\n'))
				{
					c = nxt;
					valid = true;
					read.next();

				}
				
				if (!valid)
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud value at line %u:%u: \'%c\'", read.line, read.lineChar, c);
					throw std::exception(buf);
				}

			}

			if (!enableJSON5 && c == '\n')
			{
				throw std::exception("Newlines not allowed in strings");
			}

			ss << c;

		}

		return ss.str();
	}

	std::string parseUnquotedString(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		if (!enableJSON5)
		{
			char buf[256];
			sprintf_s(buf, 256, "Invalud string literal at line %u:%u", read.line, read.lineChar);
			throw std::exception(buf);
		}

		char c = str[read.current];
		std::stringstream ss;

		if (!isASCIILetter(c))
		{
			char buf[256];
			sprintf_s(buf, 256, "Invalud value at %u:%u: \'%c\'", read.line, read.lineChar, c);
			throw std::exception(buf);
		}

		while (isASCIILetter(c = str[read.current]))
		{
			ss << c;
			read.next();

		}

		return ss.str();
	}

	std::string parseSomeString(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		char startC = str[read.current];

		if (startC == '\"' || (enableJSON5 && startC == '\''))
		{
			read.next();
			return parseStringValue(str, read, malloc, startC, enableJSON5);
		}

		return parseUnquotedString(str, read, malloc, enableJSON5);
	}

	JArray* parseJArray(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		std::vector<JValue*> vals;
		bool expectNextObj = false;

		while (true)
		{
			if (str[read.current] == ']')
			{
				if (!enableJSON5 && expectNextObj)
				{
					char buf[256];
					sprintf_s(buf, 256, "Trailing comma found at line %u:%u", read.line, read.lineChar);
					throw std::exception(buf);
				}

				read.next();
				break;
			}

			expectNextObj = false;

			skipWhitespace(str, read, enableJSON5);

			JValue* val = parseJValue(str, read, malloc, enableJSON5);

			vals.push_back(val);

			if (str[read.current] == ',')
			{
				expectNextObj = true;
				read.next();

			}

		}

		JValue** array = (JValue**)malloc->allocate(vals.size() * sizeof(void*), sizeof(void*));
		
		for (uint32_t i = 0; i < vals.size(); ++i)
		{
			array[i] = vals.at(i);

		}

		return malloc->allocArray(array, vals.size());
	}

	JObject* parseJObject(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		JObject* ret = malloc->allocObject();
		bool expectNextObj = false;

		while (true)
		{
			skipWhitespace(str, read, enableJSON5);

			if (str[read.current] == '}')
			{
				if (!enableJSON5 && expectNextObj)
				{
					char buf[256];
					sprintf_s(buf, 256, "Trailing comma found at line %u:%u'", read.line, read.lineChar);
					throw std::exception(buf);
				}

				read.next();
				break;
			}

			if (str[read.current] == ',')
			{
				char buf[256];
				sprintf_s(buf, 256, "Erroneous comma found at line %u:%u'", read.line, read.lineChar);
				throw std::exception(buf);
			}

			expectNextObj = false;

			std::string key = parseSomeString(str, read, malloc, enableJSON5);

			skipWhitespace(str, read, enableJSON5);

			if (str[read.current] != ':')
			{
				char buf[256];
				sprintf_s(buf, 256, "Invalud value at %u:%u: \'%c\'", read.line, read.lineChar, str[read.current]);
				throw std::exception(buf);
			}

			read.next();

			skipWhitespace(str, read, enableJSON5);

			JValue* val = parseJValue(str, read, malloc, enableJSON5);

			ret->setValue(key, val);

			if (str[read.current] == ',')
			{
				expectNextObj = true;
				read.next();
			}

		}

		return ret;
	}

	JObject* parseJObject(const std::string& str, JMalloc*& malloc, bool enableJSON5 = JSON5_DISABLE)
	{
		JReadData read;

		skipWhitespace(str, read, enableJSON5);

		if (str[read.current] != '{')
		{
			throw std::exception("Invalud JSON object");
		}

		read.next();

		if (malloc == nullptr)
		{
			malloc = new JMalloc(str.length());
		}

		return parseJObject(str, read, malloc, enableJSON5);
	}

	JValue* parseJValue(const std::string& str, JReadData& read, JMalloc*& malloc, bool enableJSON5)
	{
		char startC = str[read.current];

		if (isIntStart(startC, enableJSON5))
		{
			return parseJIntOrFloat(str, read, malloc, enableJSON5);
		}

		if (startC == '{')
		{
			read.next();
			return parseJObject(str, read, malloc, enableJSON5);
		}

		if (startC == '[')
		{
			read.next();
			return parseJArray(str, read, malloc, enableJSON5);
		}

		if (startC == 't' || startC == 'f')
		{
			JBool* bVal = parseJBool(str, read, malloc, enableJSON5);
			if (bVal) return bVal;
		}

		//TODO figure out what to do with this, this looks terrible
		if (startC == 'n' && str.substr(read.current, 4) == "null")
		{
			return nullptr;
		}

		std::string parsedStr = parseSomeString(str, read, malloc, enableJSON5);

		char* strMem = (char*)malloc->allocate(parsedStr.length());
		std::memcpy(strMem, parsedStr.c_str(), parsedStr.length());

		return malloc->allocString(strMem, parsedStr.length());
	}

	JValue* parseJValue(const std::string& str, JMalloc*& malloc, bool enableJSON5 = JSON5_DISABLE)
	{
		JReadData read;

		if (malloc == nullptr)
		{
			//The resulting data structure will likely be smaller than the string,
			//so allocating that much memory is a good idea to ensure it all fits.
			malloc = new JMalloc(str.length());
		}

		return parseJValue(str, read, malloc, enableJSON5);
	}

}