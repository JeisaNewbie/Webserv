#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

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

		void setSubRoot(std::string _location_path);

		int					getLocationType(void) const;
		const std::string&	getLocationPath(void) const;
		const std::string&	getSubRoot(void) const;

	private:
		int			location_type;
		std::string	location_path;
		std::string	sub_root;
};

#endif