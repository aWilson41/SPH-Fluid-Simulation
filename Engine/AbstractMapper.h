#pragma once
#include <bitset>
#include <map>

class Renderer;

// A map of properties implemented as a bitset for use as a unique key
// Good way to do branching based off properties
template<size_t T>
class PropertyMap
{
public:
	std::bitset<T> getPropertyBits() { return propertyBits; }

	// Returns assigned index in bitset
	size_t addProperty(std::string key, bool initialState = false)
	{
		indexMap[key] = incrementalKey;
		propertyBits.set(incrementalKey, initialState);
		incrementalKey++;
		return incrementalKey - 1;
	}

	void setProperty(std::string key, bool state)
	{
		propertyBits.set(indexMap[key], state);
	}

	void clear()
	{
		propertyBits.reset();
		indexMap.clear();
		incrementalKey = 0;
	}

private:
	std::bitset<T> propertyBits;
	std::map<std::string, size_t> indexMap;
	int incrementalKey = 0;
};

class AbstractMapper
{
public:
	virtual void update() = 0;

	virtual void draw(Renderer * ren) = 0;

protected:
	// Holds properties in a bitset + map
	PropertyMap<32> propertyMap;
};