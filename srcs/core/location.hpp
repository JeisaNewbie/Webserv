#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

class Location {
	public:
		Location(void);
		Location(std::string _block_path);
		Location(const Location& obj);
		~Location(void);

		Location& operator =(const Location& src);

		void setStaticPath(std::string _block_path);
		void setCgiPath(std::string _cgi_path);

		const std::string& getBlockPath(void) const;
		const std::string& getStaticPath(void) const;
		const std::string& getCgiPath(void) const;

	private:

		std::string	block_path;
		std::string	static_path;
		std::string	cgi_path;
};

#endif