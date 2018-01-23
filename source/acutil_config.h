/*
   AngelCode Tool Box Library
   Copyright (c) 2008-2016 Andreas Jonsson
  
   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.
  
   Andreas Jonsson
   andreas@angelcode.com
*/

// 2014-06-14 Changed LoadConfigFile to take std::string instead of char*
// 2011-08-11 Added GetNumberOfAttributes and GetNameOfAttribute
// 2009-06-18 Added const correctness
// 2009-03-11 Changed index to unsigned to avoid crash on negative values
// 2008-12-07 Added GetAttrAsBool
// 2008-11-13 Added support for multiple attributes of the same name

#ifndef ACUTIL_CONFIG_H
#define ACUTIL_CONFIG_H

#include <string>
#include <map>
#include <vector>

namespace acUtility
{

//! \brief Load configurations from a text file with simple syntax
//! 
//! The CConfig class can be used to load configuration from a text file 
//! with a simple syntax that resembles Windows .ini files.
//!
//! The configured attributes are stored as name/value pairs. The same name
//! can be used multiple times to form an array of values.
//!
//! Example configuration:
//!
//! <pre>
//!   # Different attributes and types
//!   an_integer=1
//!   a_float=3.14
//!   a_string=hello world
//!   a_hex_value=hBAADF00D
//!
//!   # An array of values can be formed by repeating the same name
//!   element=1
//!   element=2
//!   element=3
//! </pre>
class CConfig
{
public:
	CConfig();
	~CConfig();

	int LoadConfigFile(const std::string &file);

	// Returns the number of different attributes
	int  GetNumberOfAttributes() const;

	// Return name of attribute by index
	bool GetNameOfAttribute(int index, std::string &name) const;

	// Returns the number of entries for an attribute
	int  GetAttrCount(const char *name) const;

	// Get a value of an entry for an attribute. Returns true if the entry is found
	bool GetAttrAsInt(const char *name, int &value, unsigned int index = 0, int defaultValue = 0) const;
	bool GetAttrAsString(const char *name, std::string &value, unsigned int index = 0, const std::string &defaultValue = "") const;
	bool GetAttrAsFloat(const char *name, float &value, unsigned int index = 0, float defaultValue = 0) const;
	bool GetAttrAsDouble(const char *name, double &value, unsigned int index = 0, double defaultValue = 0) const;
	bool GetAttrAsBool(const char *name, bool &value, unsigned int index = 0, bool defaultValue = false) const;

protected:
	std::map<std::string, std::vector<std::string> > attrMap;
};

}

#endif
