#ifndef Timeout_HPP
# define Timeout_HPP

# include <ctime>

class Event;

class Timeout {
	public:
		Timeout(void);
		Timeout(const Timeout& src);
		~Timeout(void);

		Timeout& operator =(const Timeout& src);

		void		setSavedTime(void);

		bool		checkTimeout(size_t timeout);

	private:
		time_t	saved_time;

};

#endif