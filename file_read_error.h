#pragma once
#include <exception>
#include <string>
class file_read_error : public std::runtime_error
{
public:
	file_read_error(std::string input);
	virtual ~file_read_error();
	virtual std::string what();
private:
	typedef runtime_error super;
};


