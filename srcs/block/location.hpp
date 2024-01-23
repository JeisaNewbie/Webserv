#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

# define METHOD_GET		0x00000001
# define METHOD_POST	0x00000010
# define METHOD_DELETE	0x00000100

enum loc_type {
	LOC_DEFAULT,
	LOC_ERROR,
	LOC_CGI
};

class Location {
	public:
		Location(int location_type, std::string _location_path);
		Location(const Location& obj);
		~Location(void);

		Location& operator =(const Location& src);

		void 						setSubRoot(std::string _location_path);
		void 						setAllowedMethod(int _allowed_method);
		void 						setAutoIndex(int _autoindex);

		int							getLocationType(void) const;
		const std::string&			getLocationPath(void) const;
		const std::string&			getSubRoot(void) const;
		int							getAllowedMethod(void) const;
		int							getAutoIndex(void) const;
		std::vector<std::string>&	getIndex(void);

	private:
		int							location_type;
		std::string					location_path;
		std::string					sub_root;
		int							allowed_method;
		int							autoindex;
		std::vector<std::string>	index;
};

#endif