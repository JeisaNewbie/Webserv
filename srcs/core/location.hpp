#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

class Location {
	public:
		Location(std::string _location_path);
		Location(const Location& obj);
		~Location(void);

		Location& operator =(const Location& src);

		void setStaticPath(std::string _location_path);

		const std::string& getBlockPath(void) const;
		const std::string& getStaticPath(void) const;

	private:
		Location(void);

		std::string	location_path;
		std::string	sub_root;
};

#endif