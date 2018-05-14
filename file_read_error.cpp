#include "file_read_error.h"




file_read_error::file_read_error(std::string location):super(location)
{
}

file_read_error::~file_read_error()
{
}

std::string file_read_error::what()
{
	std::string loc = dynamic_cast<super*>(this)->what();
	return "Could not read file at " + loc + ".";
}
