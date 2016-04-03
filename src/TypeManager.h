/*
 * Copyright 2015 Gary R. Van Sickle (grvs@users.sourceforge.net).
 *
 * This file is part of UniversalCodeGrep.
 *
 * UniversalCodeGrep is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU General Public License as published by the Free
 * Software Foundation.
 *
 * UniversalCodeGrep is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * UniversalCodeGrep.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file */

#ifndef TYPEMANAGER_H_
#define TYPEMANAGER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

/**
 * Class which manages the file types which are to be scanned.
 */
class TypeManager
{
public:
	TypeManager();
	virtual ~TypeManager();

	/**
	 * Determine if the file with the given path and name should be scanned based on the
	 * enabled file types.
	 *
	 * @param path
	 * @param name
	 * @return
	 */
	bool FileShouldBeScanned(const std::string &name) const;

	/**
	 * Add the given file type to the types which will be scanned.  For handling the
	 * --type= command line param.  The first time this function is called, all currently-
	 * active types are removed, and only the given type (and types given subsequently)
	 * will be searched.
	 *
	 * @param type_name  Name of the type.
	 * @return true on success, false if no such type.
	 */
	bool type(const std::string &type_name);

	bool notype(const std::string &type_name);

	/**
	 * Determines if #type is in the m_active_type_map as a type name.
	 *
	 * @param type  Name of the type to check.
	 * @return true if #type names a type, false otherwise.
	 */
	bool IsType(const std::string &type) const;

	void TypeAddIs(const std::string &type, const std::string &name);

	void TypeAddExt(const std::string &type, const std::string &ext);

	/**
	 * Deletes #type from the m_active_type_map.
	 *
	 * @param type  Name of the type to delete.
	 * @return true if #type named a type, false if it did not.
	 */
	bool TypeDel(const std::string &type);

	void CompileTypeTables();

	void PrintTypesForHelp(std::ostream &s) const;

private:

	/// Flag to keep track of the first call to type().
	bool m_first_type_has_been_seen = { false };

	/// Map of file type names to the associated filename filters.
	/// This contains both built-in types and user-defined types created by TypeAdd*().
	/// Both built-in and user types can be removed with TypeDel().
	std::map<std::string, std::vector<std::string>> m_builtin_and_user_type_map;

	/// Map of file type names to the associated filename filters.
	/// This is the active map, which will eventually be compiled into the
	/// hash tables used at filename-scanning time.
	std::map<std::string, std::vector<std::string>> m_active_type_map;

	/// Map of file type filters which have been removed by calls to notype().
	/// Maps to the type(s) which were specified in the notype() call(s).
	std::unordered_multimap<std::string, std::string> m_removed_type_filters;

	/// File extensions which will be examined.  Maps to file type.
	std::unordered_multimap<std::string, std::string> m_include_extensions;

	/// Literal filenames which will be examined.  Maps to file type.
	std::unordered_multimap<std::string, std::string> m_included_literal_filenames;

	/// Map of the regexes to try to match to the first line of the file (key) to
	/// the file type (value).
	std::unordered_multimap<std::string, std::string> m_included_first_line_regexes;
};

#endif /* TYPEMANAGER_H_ */
