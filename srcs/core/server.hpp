#ifndef SERVER_HPP
# define SERVER_HPP

# include <list>
# include "location.hpp"

class Server {
	public:
		Server(void);
		Server(const Server& obj);
		~Server(void);

		Server& operator =(const Server& src);

		void						setPort(uint32_t _port);
		void						setDomain(std::string _domain);
		void						setErrorPage(std::string _error_page);

		uint32_t					getPort(void) const;
		const std::string&			getDomain(void) const;
		const std::string&			getErrorPage(void) const;
		std::list<Location>&		getLocationList(void);
		const std::list<Location>&	getLocationListConst(void) const;

	private:
		uint32_t			port;
		std::string			domain;
		std::string			error_page;

		std::list<Location> location_list;
};

#endif