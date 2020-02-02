#ifndef RENDERER_EXCEPTION_H
#define RENDERER_EXCEPTION_H
#include <exception>
#include <string>

class RendererException : public std::exception
{
private:
	std::string m_error;
public:
	explicit RendererException(std::string error) : m_error(std::string("Vulkan: ") + std::move(error)) {}
	RendererException(const RendererException& exception) noexcept {
		m_error = exception.m_error;
	}
	const char* what() const noexcept override {
		return m_error.c_str();
	}
	virtual ~RendererException() {}
};

#endif //RENDERER_EXCEPTION_H

