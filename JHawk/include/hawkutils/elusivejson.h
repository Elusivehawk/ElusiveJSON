
//NOTE: Define ELUSIVEJSON_DISABLE_JSON5 to disable JSON 5 support

#ifndef ELUSIVEJSON_DISABLE_JSON5
#define ELUSIVEJSON_ENABLE_JSON5
#endif

#pragma once

#include <cstdio>
#include <limits>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#define NOT_IMPL_YET {throw new function_not_impl();}

namespace ElusiveJSON
{
	class function_not_impl : public std::logic_error
	{
	public:
		function_not_impl() : std::logic_error("Method or function not yet implemented") {}
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
		std::string value;
	public:
		JString(std::string str) : value(str) {}

		std::string strValue()
		{
			return value;
		}

		std::string toString(bool pretty = false, int scope = 0)
		{
			std::stringstream ss;
			
			ss << '\"';
			ss << value;
			ss << '\"';

			return ss.str();
		}

	};

	class JObject : public JValue
	{
	private:
		std::unordered_map<std::string, JValue*> values = {};
	public:
		JObject() {}

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
		JArray(size_t len) : length(len)
		{
			values = new JValue * [len];

		}

		~JArray()
		{
			delete[] values;
		}

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

	void skipWhitespace(std::string str, uint32_t& index)
	{
		while (true)
		{
#ifdef ELUSIVEJSON_ENABLE_JSON5
			if (str[index] == '/')
			{
				if (str[index + 1] == '/')
				{
					index += 2;
					while (str[index] != '\n') { ++index; }
					++index;

				}
				else if (str[index + 1] == '*')
				{
					index += 2;
					while (str[index] != '*' && str[index + 1] != '/') { ++index; }
					index += 2;

				}
				else
				{
					//FIXME
					throw std::exception("Invalud syntax");
				}

			}

#endif

			switch (str[index])
			{
				case ' ':
				case '\n':
				case '\r':
				case '\t': ++index; continue;
				default: break;
			}

			break;
		}

	}

	inline bool isInt(char c)
	{
		return (c >= '0' && c <= '9');
	}

	inline bool isHexInt(char c)
	{
		return isInt(c) || (c >= 'A' && c <= 'F');
	}

	bool isIntStart(char c)
	{
		return isInt(c) || c == '-'
			/*JSON 5 support (lord I regret this)*/ 
#ifdef ELUSIVEJSON_ENABLE_JSON5
			|| c == '.' || c == '+'
#endif
			;
	}

	bool isASCIILetter(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	//Core API forward declarations
	JBool* parseJBool(std::string str, uint32_t& start);
	JValue* parseJIntOrFloat(std::string str, uint32_t& start);
	JArray* parseJArray(std::string str, uint32_t& start);
	JObject* parseJObject(std::string str, uint32_t& start);
	JValue* parseJValue(std::string str, uint32_t& start);

	std::string parseStringValue(std::string str, uint32_t& start)
	{
		std::stringstream ret;

		while (true)
		{
			char c = str[start];
			++start;

			if (c == '\"')
			{
				break;
			}

#ifdef ELUSIVEJSON_ENABLE_JSON5
			if (c == '\'')
			{
				break;
			}

#endif

			if (c == '\\')
			{
				switch (str[start]/*now points to the next char*/)
				{
					case '\"': c = '\"'; ++start; break;// In both of these cases, it could confuse an escaped quote for an end quote
#ifdef ELUSIVEJSON_ENABLE_JSON5
					case '\'': c = '\''; ++start; break;
					case '\n':
#endif
					case 'n': c = '\n'; break;
					case 'u': {
						for (int i = 1; i <= 4; ++i)
						{
							if (!isHexInt(str[start + i]))
							{
								char buf[256];
								sprintf_s(buf, 256, "Malformed UTF-8 string literal at %u", start);
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
					default: {
						char buf[256];
						sprintf_s(buf, 256, "Invalud value at %u: \'%c\'", start, c);
						throw std::exception(buf);
					}
				}
			}
#ifdef	ELUSIVEJSON_DISABLE_JSON5
			if (c == '\n')
			{
				throw std::exception("Newlines not allowed in strings");
			}
#endif

			ret << c;

		}

		return ret.str();
	}

	std::string parseUnquotedString(std::string str, uint32_t& start)
	{
		char c = str[start];
		std::stringstream ret;

		if (!isASCIILetter(c))
		{
			char buf[256];
			sprintf_s(buf, 256, "Invalud value at %u: \'%c\'", start, c);
			throw std::exception(buf);
		}

		while (isASCIILetter(c = str[start]))
		{
			ret << c;
			++start;

		}

		return ret.str();
	}

	std::string parseSomeString(std::string str, uint32_t& start)
	{
		char startC = str[start];

		if (startC == '\"'
#ifdef ELUSIVEJSON_ENABLE_JSON5
			|| startC == '\''
#endif
		)
		{
			++start;
			return parseStringValue(str, start);
		}

#ifdef ELUSIVEJSON_DISABLE_JSON5
		char buf[256];
		sprintf_s(buf, 256, "Invalud string literal at %u", start);
		throw std::exception(buf);
#else
		return parseUnquotedString(str, start);
#endif
	}

	JBool* parseJBool(std::string str, uint32_t& start)
	{
		if (str.substr(start, 4) == "true")
		{
			start += 4;
			return new JBool(true);
		}
		else if (str.substr(start, 5) == "false")
		{
			start += 5;
			return new JBool(false);
		}

		return nullptr;
	}

	JValue* parseJIntOrFloat(std::string str, uint32_t& start)
	{
		std::stringstream ss;
		bool isHex = false;
		bool isFloat = false;
		bool hasExponent = false;
		int consumed = 0;//For sanity checking against non-JSON 5 ints

#ifdef ELUSIVEJSON_ENABLE_JSON5
		if (str[start] == '∞')
		{
			return new JFloat(std::numeric_limits<float>::infinity());
		}

		if (str[start] == '-' && str[start + 1] == '∞')
		{
			return new JFloat(std::numeric_limits<float>::infinity() * -1.0f);
		}

		if (str.substr(start, 3) == "NaN")
		{
			return new JFloat(NAN);
		}

		if (str[start] == '0' && str[start + 1] == 'x')
		{
			isHex = true;
			ss << std::hex;
			ss << str[start] << str[start + 1];
			start += 2;
			consumed += 2;

		}
#endif
		
		while (true)
		{
			char c = str[start];

			if (isInt(c) || (isHex && isHexInt(c)) /*Doesn't need to be checked against ELUSIVEJSON_DISABLE_JSON5 because of the isHex flag*/)
			{
				ss << c;
				++start;
				++consumed;
				continue;
			}
			else if (c == '.')
			{
				if (isHex || isFloat || hasExponent)
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud char found in int at %u: \'%c\'", start, c);
					throw std::exception(buf);
				}

#ifdef ELUSIVEJSON_DISABLE_JSON5
				if (consumed == 0 || !isInt(str[start + 1]))
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud char found in int at %u: \'%c\'", start, c);
					throw std::exception(buf);
				}
#endif

				isFloat = true;
				ss << c;
				++start;
				++consumed;
				continue;
			}
			else if (c == 'e' || c == 'E')
			{
				hasExponent = true;
				ss << c;
				++start;
				++consumed;

				char sign = str[start];

				if (sign != '-' && sign != '+' && !isInt(c))//Apparently signage is optional
				{
					char buf[256];
					sprintf_s(buf, 256, "Invalud exponent signage in int at %u: \'%c\'", start, c);
					throw std::exception(buf);
				}

				ss << c;
				++start;
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
			return new JFloat(f);
		}

		int i;
		ss >> i;

		return new JInt(i);
	}

	JArray* parseJArray(std::string str, uint32_t& start)
	{
		std::vector<JValue*> vals;
		bool expectNextObj = false;

		while (true)
		{
			if (str[start] == ']')
			{
#ifdef ELUSIVEJSON_DISABLE_JSON5
				if (expectNextObj)
				{
					char buf[256];
					sprintf_s(buf, 256, "Trailing comma found at %u'", start);
					throw std::exception(buf);
				}
#endif

				++start;
				break;
			}

			expectNextObj = false;

			skipWhitespace(str, start);

			JValue* val = parseJValue(str, start);

			vals.push_back(val);

			if (str[start] == ',')
			{
				expectNextObj = true;
				++start;
			}

		}

		JArray* ret = new JArray(vals.size());

		for (size_t i = 0; i < vals.size(); ++i)
		{
			ret->setValue(i, vals.at(i));

		}

		return ret;
	}

	JObject* parseJObject(std::string str, uint32_t& start)
	{
		JObject* ret = new JObject();
		bool expectNextObj = false;

		while (true)
		{
			skipWhitespace(str, start);

			if (str[start] == '}')
			{
#ifdef ELUSIVEJSON_DISABLE_JSON5
				if (expectNextObj)
				{
					char buf[256];
					sprintf_s(buf, 256, "Trailing comma found at %u'", start);
					throw std::exception(buf);
				}
#endif

				++start;
				break;
			}

			if (str[start] == ',')
			{
				char buf[256];
				sprintf_s(buf, 256, "Erroneous comma found %u'", start);
				throw std::exception(buf);
			}

			expectNextObj = false;

			std::string key = parseSomeString(str, start);

			skipWhitespace(str, start);

			if (str[start] != ':')
			{
				char buf[256];
				sprintf_s(buf, 256, "Invalud value at %u: \'%c\'", start, str[start]);
				throw std::exception(buf);
			}

			++start;

			skipWhitespace(str, start);

			JValue* val = parseJValue(str, start);

			ret->setValue(key, val);

			if (str[start] == ',')
			{
				expectNextObj = true;
				++start;
			}

		}

		return ret;
	}

	JObject* parseJObject(std::string str)
	{
		uint32_t start = 0;

		skipWhitespace(str, start);

		if (str[start] != '{')
		{
			throw std::exception("Invalud JSON");
		}

		++start;

		return parseJObject(str, start);
	}

	JValue* parseJValue(std::string str, uint32_t& start)
	{
		char startC = str[start];

		if (isIntStart(startC))
		{
			return parseJIntOrFloat(str, start);
		}

		if (startC == '{')
		{
			++start;
			return parseJObject(str, start);
		}

		if (startC == '[')
		{
			++start;
			return parseJArray(str, start);
		}

		if (startC == 't' || startC == 'f')
		{
			JBool* bVal = parseJBool(str, start);
			if (bVal) return bVal;
		}

		if (startC == 'n' && str.substr(start, 4) == "null")
		{
			return nullptr;
		}

		return new JString(parseSomeString(str, start));
	}

	JValue* parseJValue(std::string str)
	{
		uint32_t start = 0;
		return parseJValue(str, start);
	}

}