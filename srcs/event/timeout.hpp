#ifndef Timeout_HPP
# define Timeout_HPP

# include <ctime>

class Timeout {
	public:
		Timeout(void);
		Timeout(const Timeout& src);
		~Timeout(void);

		Timeout& operator =(const Timeout& src);

		void	setSavedTime(void);
		
		bool	checkTimeout(uint32_t timeout);

	private:
		time_t	saved_time;
};

#endif