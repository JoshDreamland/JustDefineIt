#ifndef JDI_TESTING_ERROR_HANDLER_h
#define JDI_TESTING_ERROR_HANDLER_h

#include <API/error_reporting.h>

namespace {

class ErrorConstitutesFailure : public jdi::error_handler {
  void error(std::string_view err, std::string_view filename, int line, int pos) final {
    ADD_FAILURE() << "Underlying code reported an error: " << err
                  << " (at " << filename << ":" << line << ":" << pos << ")";
  }
  void warning(std::string_view err, std::string_view filename, int line, int pos) final {
    ADD_FAILURE() << "Underlying code reported a warning: " << err
                  << " (at " << filename << ":" << line << ":" << pos << ")";
  }
} *error_constitutes_failure = new ErrorConstitutesFailure;

}  // namespace

#endif  // JDI_TESTING_ERROR_HANDLER_h
