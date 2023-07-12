#pragma once

#include <typeinfo>
#include <vector>
#include <string>

class Key {
private:
	std::size_t hash;
	const std::type_info & type;
	std::vector<char> data;

public:
	Key(const std::string & str) :
			hash(std::hash<std::string>{}(str)),
			type(typeid(std::string)),
			data(str.begin(), str.end())
	{

	}

	template<typename T>
	Key(const T & t) :
			hash(std::hash<T>{}(t)),
			type(typeid(T)),
			data(reinterpret_cast<const char*>(&t), reinterpret_cast<const char*>(&t)+sizeof(T))
	{
	}

	std::size_t getHash() const {
		return hash;
	}

	bool operator==(const Key & rhs) const {
		return 
			type == rhs.type &&
			data == rhs.data;
	}
};

template<>
struct std::hash<Key>
{
	std::size_t operator()(const Key & key) const {
		return key.getHash();
	}
};
