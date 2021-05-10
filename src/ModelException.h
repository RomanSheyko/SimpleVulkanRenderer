#ifndef ModelException_h
#define ModelException_h
#include <exception>
#include <string>

class ModelException : public std::exception
{
private:
    std::string m_error;
public:
    explicit ModelException(std::string error) : m_error(std::string("Vulkan: ") + std::move(error)) {}
    ModelException(const ModelException& exception) noexcept {
        m_error = exception.m_error;
    }
    const char* what() const noexcept override {
        return m_error.c_str();
    }
    virtual ~ModelException() {}
};

#endif /* ModelException_h */
