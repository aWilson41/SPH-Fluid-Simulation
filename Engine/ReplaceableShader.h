#pragma once
#include <list>
#include <map>
#include <string>

class ReplaceableStringSet
{
public:
	// returns location it assigned
	int addReplacementString(std::string attribName, std::string replacementString)
	{
		unsigned int location = getUniqueLocation();
		locations[attribName] = location;
		replacementStrings[location] = replacementString;// prefix + " " + attribName + ";";
		status[location] = false;
		return location;
	}
	// Remove by by name
	void removeReplacementString(std::string attribName)
	{
		unsigned int location = locations[attribName];
		locations.erase(attribName);
		replacementStrings.erase(location);
		status.erase(location);
	}
	// Remove by location
	void removeReplacementString(unsigned int location)
	{
		// Perform a search (removing doesn't happen that much, otherwise I would define it's inverse)
		std::string key = "";
		for (std::map<std::string, unsigned int>::iterator i = locations.begin(); i != locations.end(); i++)
		{
			if (location == i->second)
			{
				key = i->first;
				break;
			}
		}
		if (key != "")
		{
			locations.erase(key);
			replacementStrings.erase(location);
			status.erase(location);
		}
	}
	// Sets whether a current replacement string is active
	void setReplacementString(unsigned int location, bool active)
	{
		// Check if it contains the the attribute to avoid adding it
		if (status.count(location) == 1)
			status[location] = active;
	}
	// Sets whether a current replacement string is active
	void setReplacementString(std::string attribName, bool active) { setReplacementString(locations[attribName], active); }


	unsigned int getLocation(std::string attribName) { return locations[attribName]; }

private:
	unsigned int getUniqueLocation()
	{
		unsigned int i = 0;
		bool notFound = true;
		while (notFound)
		{
			if (status.count(i) == 0)
				notFound = false;
			else
				i++;
		}
		return i;
	}

public:
	// Given a name returns the location (name, location)
	std::map<std::string, unsigned int> locations;
	// Given a location gives the string to insert (location, string replacement)
	std::map<unsigned int, std::string> replacementStrings;
	// layout location, active status (using or not)
	std::map<unsigned int, bool> status;
};

// Implements a replaceable shader
class ReplaceableShader
{
public:
	//void addFunction(std::string funcName, std::string returnType, std::string funcStr);
	void setShaderBody(std::string shaderBody) { ReplaceableShader::shaderBody = shaderBody; }
	// Put before uniforms/attributes/etc. I often use it for structs
	void setShaderHeader(std::string shaderHeader) { ReplaceableShader::shaderHeader = shaderHeader; }

	// returns location it assigned
	int addStruct(std::string structName, std::string replacementString) { return structs.addReplacementString(structName, replacementString); }
	void removeStruct(std::string structName) { structs.removeReplacementString(structName); }
	void removeStruct(unsigned int location) { structs.removeReplacementString(location); }
	// Sets whether a current struct is active
	void setStruct(unsigned int location, bool active) { structs.setReplacementString(location, active); }
	// Sets whether a current struct is active
	void setStruct(std::string structName, bool active) { structs.setReplacementString(structName, active); }
	unsigned int getStructLocation(std::string structName) { return structs.getLocation(structName); }

	// returns location it assigned
	int addInAttribute(std::string attribName, std::string replacementString) { return inAttributes.addReplacementString(attribName, replacementString); }
	void removeInAttribute(std::string attribName) { inAttributes.removeReplacementString(attribName); }
	void removeInAttribute(unsigned int location) { inAttributes.removeReplacementString(location); }
	// Sets whether a current inAttribute is active
	void setInAttribute(unsigned int location, bool active) { inAttributes.setReplacementString(location, active); }
	// Sets whether a current inAttribute is active
	void setInAttribute(std::string attribName, bool active) { inAttributes.setReplacementString(attribName, active); }
	unsigned int getInAttributeLocation(std::string attribName) { return inAttributes.getLocation(attribName); }

	// returns location it assigned
	int addOutAttribute(std::string attribName, std::string replacementString) { return inAttributes.addReplacementString(attribName, replacementString); }
	void removeOutAttribute(std::string attribName) { inAttributes.removeReplacementString(attribName); }
	void removeOutAttribute(unsigned int location) { inAttributes.removeReplacementString(location); }
	// Sets whether a current outAttribute is active
	void setOutAttribute(unsigned int location, bool active) { inAttributes.setReplacementString(location, active); }
	// Sets whether a current outAttribute is active
	void setOutAttribute(std::string attribName, bool active) { inAttributes.setReplacementString(attribName, active); }
	unsigned int getOutAttributeLocation(std::string attribName) { return inAttributes.getLocation(attribName); }

	// returns location it assigned
	int addUniform(std::string uniformName, std::string replacementString) { return outAttributes.addReplacementString(uniformName, replacementString); }
	void removeUniform(std::string uniformName) { outAttributes.removeReplacementString(uniformName); }
	void removeUniform(unsigned int location) { outAttributes.removeReplacementString(location); }
	// Sets whether a current uniform is active
	void setUniform(unsigned int location, bool active) { outAttributes.setReplacementString(location, active); }
	// Sets whether a current uniform is active
	void setUniform(std::string uniformName, bool active) { outAttributes.setReplacementString(uniformName, active); }
	unsigned int getUniformLocation(std::string uniformName) { return outAttributes.getLocation(uniformName); }

	
	std::string update()
	{
		shaderStr = "#version 460 core" + shaderHeader + "\n\n";
		// Add structs
		for (std::map<unsigned int, std::string>::iterator i = structs.replacementStrings.begin();
			i != structs.replacementStrings.end(); i++)
		{
			shaderStr += i->second + '\n';
		}
		shaderStr += '\n';
		// Add uniforms
		for (std::map<unsigned int, std::string>::iterator i = uniforms.replacementStrings.begin();
			i != uniforms.replacementStrings.end(); i++)
		{
			shaderStr += "layout(binding = " + std::to_string(i->first) + ") " + i->second + '\n';
		}
		shaderStr += '\n';
		// Add in attribute definitions
		for (std::map<unsigned int, std::string>::iterator i = inAttributes.replacementStrings.begin();
			i != inAttributes.replacementStrings.end(); i++)
		{
			shaderStr += "layout(location = " + std::to_string(i->first) + ") " + i->second + '\n';
		}
		shaderStr += '\n';
		// Add out attribute definitions
		for (std::map<unsigned int, std::string>::iterator i = outAttributes.replacementStrings.begin();
			i != outAttributes.replacementStrings.end(); i++)
		{
			shaderStr += i->second + '\n';
		}

		shaderStr += '\n' + "void main()\n{\n" + shaderBody + "\n}";
		return shaderStr;
	}
	std::string getShaderStr() { return shaderStr; }

private:
	ReplaceableStringSet structs;
	ReplaceableStringSet uniforms;
	ReplaceableStringSet inAttributes;
	ReplaceableStringSet outAttributes;

	std::string shaderHeader = "";
	std::string shaderBody = "";
	std::string shaderStr = "";
};