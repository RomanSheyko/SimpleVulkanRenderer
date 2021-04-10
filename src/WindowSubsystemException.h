#ifndef _WINDOWSUBSYSTEM_EXCEPTION_H
#define _WINDOWSUBSYSTEM_EXCEPTION_H
#include <exception>
#include <string>

class WindowSubsystemException : public std::exception
{
private:
	std::string m_error;
public:
	explicit WindowSubsystemException(std::string error) : m_error(std::string("Vulkan: ") + std::move(error)) {}
	WindowSubsystemException(const WindowSubsystemException& exception) noexcept {
		m_error = exception.m_error;
	}
	const char* what() const noexcept override {
		return m_error.c_str();
	}
	virtual ~WindowSubsystemException() {}
};

#endif //_WINDOWSUBSYSTEM_EXCEPTION_H
