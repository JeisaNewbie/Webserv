#ifndef SERVER_HPP
# define SERVER_HPP

# include "location.hpp"

# include <list>
# include <iostream>

class Server {
	public:
		Server(void);
		Server(const Server& obj);
		~Server(void);

		Server& operator =(const Server& src);

		void						setPort(uint32_t _port);
		void						setDomain(std::string _domain);

		uint32_t					getPort(void) const;
		const std::string&			getDomain(void) const;
		std::list<Location>&		getLocationList(void);
		const std::list<Location>&	getLocationListConst(void) const;

	private:
		uint32_t			port;
		std::string			domain;

		std::list<Location> location_list;
};

#endif