#ifndef TMS_AUTHENTICATION_MANAGER_H
#define TMS_AUTHENTICATION_MANAGER_H

#include "User.h"
#include <memory>
#include <string>

namespace tms {

class AuthenticationManager {
private:
    std::shared_ptr<User> m_currentUser;

public:
    AuthenticationManager();

    bool registerPassenger(const std::string& username, const std::string& rawPassword,
                           const std::string& fullName, const std::string& email,
                           const std::string& phone, const std::string& address);

    std::shared_ptr<User> login(const std::string& username, const std::string& rawPassword, UserRole expectedRole);
    void logout();

    bool isLoggedIn() const { return m_currentUser != nullptr; }
    std::shared_ptr<User> getCurrentUser() const { return m_currentUser; }
    UserRole getCurrentRole() const;
};

} // namespace tms

#endif // TMS_AUTHENTICATION_MANAGER_H
