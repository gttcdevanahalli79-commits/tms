#ifndef TMS_USER_H
#define TMS_USER_H

#include "Common.h"
#include <string>
#include <iostream>
#include <memory>

namespace tms {

/**
 * @brief Abstract Base Class demonstrating Abstraction, Encapsulation, and Polymorphism.
 */
class User {
protected:
    int m_userId;
    std::string m_username;
    std::string m_passwordHash;
    std::string m_fullName;
    std::string m_email;
    std::string m_phone;
    UserRole m_role;

public:
    User(int id, std::string username, std::string passwordHash,
         std::string fullName, std::string email, std::string phone, UserRole role);
    virtual ~User() = default;

    // Getters
    int getUserId() const { return m_userId; }
    const std::string& getUsername() const { return m_username; }
    const std::string& getPasswordHash() const { return m_passwordHash; }
    const std::string& getFullName() const { return m_fullName; }
    const std::string& getEmail() const { return m_email; }
    const std::string& getPhone() const { return m_phone; }
    UserRole getRole() const { return m_role; }

    // Setters
    void setFullName(const std::string& name) { m_fullName = name; }
    void setEmail(const std::string& email) { m_email = email; }
    void setPhone(const std::string& phone) { m_phone = phone; }
    void setPasswordHash(const std::string& hash) { m_passwordHash = hash; }

    // Pure virtual functions (Polymorphism & Abstraction)
    virtual void displayMenu() const = 0;
    virtual void printProfile() const = 0;
    virtual bool canPerformAction(const std::string& action) const = 0;

    // Password verification
    bool verifyPassword(const std::string& rawPassword) const;
    static std::string hashPassword(const std::string& rawPassword);
};

/**
 * @brief Derived Class for System Administrators with elevated privileges.
 */
class Admin : public User {
public:
    Admin(int id, std::string username, std::string passwordHash,
          std::string fullName, std::string email, std::string phone);

    void displayMenu() const override;
    void printProfile() const override;
    bool canPerformAction(const std::string& action) const override;
};

/**
 * @brief Derived Class for Railway Operators managing daily train runs and manifests.
 */
class Operator : public User {
private:
    std::string m_assignedZone;

public:
    Operator(int id, std::string username, std::string passwordHash,
             std::string fullName, std::string email, std::string phone,
             std::string zone = "Central");

    const std::string& getAssignedZone() const { return m_assignedZone; }
    void setAssignedZone(const std::string& zone) { m_assignedZone = zone; }

    void displayMenu() const override;
    void printProfile() const override;
    bool canPerformAction(const std::string& action) const override;
};

/**
 * @brief Derived Class for Passengers booking journeys and viewing tickets.
 */
class Passenger : public User {
private:
    std::string m_address;
    int m_loyaltyPoints;

public:
    Passenger(int id, std::string username, std::string passwordHash,
              std::string fullName, std::string email, std::string phone,
              std::string address = "", int points = 0);

    const std::string& getAddress() const { return m_address; }
    int getLoyaltyPoints() const { return m_loyaltyPoints; }
    void addLoyaltyPoints(int pts) { m_loyaltyPoints += pts; }

    void displayMenu() const override;
    void printProfile() const override;
    bool canPerformAction(const std::string& action) const override;
};

} // namespace tms

#endif // TMS_USER_H
