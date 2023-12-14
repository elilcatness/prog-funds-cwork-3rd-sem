#pragma once
#include <iostream>

using namespace std;

class BaseException : public exception {
protected:
	string reason;
public:
	BaseException(const string reason) : exception() {
		this->reason = reason;
	}
	const char* what() const throw() {
		return reason.c_str();
	}
};

class FileException : public BaseException {
public:
	FileException(const string reason) : BaseException(reason) {}
};

class InvalidFileExtension : public FileException {
public:
	InvalidFileExtension(const string reason) : FileException(reason) {}
};

class InvalidFileFormat : public FileException {
public:
	InvalidFileFormat(const string reason) : FileException(reason) {}
};

class EmptyInputException : public BaseException {
public:
	EmptyInputException(const string reason) : BaseException(reason) {}
};
