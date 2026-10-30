#ifndef TMS_STATION_H
#define TMS_STATION_H

#include <string>
#include <iostream>

namespace tms {

class Station {
private:
    int m_stationId;
    std::string m_code;            // e.g., SBC, YPR, MYS, UBL
    std::string m_name;            // e.g., KSR Bengaluru City
    std::string m_city;            // e.g., Bengaluru
    std::string m_district;        // e.g., Bengaluru Urban
    std::string m_state;           // Karnataka
    std::string m_railwayZone;     // e.g., South Western Railway (SWR), Southern Railway (SR), Konkan Railway (KR), Central Railway (CR), South Central Railway (SCR)
    std::string m_railwayDivision; // e.g., Bengaluru, Mysuru, Hubballi, Karwar, Palakkad, Solapur, Guntakal
    int m_platformsCount;
    double m_latitude;
    double m_longitude;

public:
    Station(int id, std::string code, std::string name, std::string city,
            std::string district, std::string state = "Karnataka",
            std::string zone = "South Western Railway", std::string division = "Bengaluru",
            int platforms = 4, double lat = 0.0, double lon = 0.0);

    // Backward-compatible constructor
    Station(int id, std::string code, std::string name, std::string city,
            std::string state, int platforms);

    // Getters
    int getStationId() const { return m_stationId; }
    const std::string& getCode() const { return m_code; }
    const std::string& getName() const { return m_name; }
    const std::string& getCity() const { return m_city; }
    const std::string& getDistrict() const { return m_district; }
    const std::string& getState() const { return m_state; }
    const std::string& getRailwayZone() const { return m_railwayZone; }
    const std::string& getRailwayDivision() const { return m_railwayDivision; }
    int getPlatformsCount() const { return m_platformsCount; }
    double getLatitude() const { return m_latitude; }
    double getLongitude() const { return m_longitude; }

    // Setters
    void setCode(const std::string& code) { m_code = code; }
    void setName(const std::string& name) { m_name = name; }
    void setCity(const std::string& city) { m_city = city; }
    void setDistrict(const std::string& district) { m_district = district; }
    void setState(const std::string& state) { m_state = state; }
    void setRailwayZone(const std::string& zone) { m_railwayZone = zone; }
    void setRailwayDivision(const std::string& division) { m_railwayDivision = division; }
    void setPlatformsCount(int count) { m_platformsCount = count; }
    void setCoordinates(double lat, double lon) { m_latitude = lat; m_longitude = lon; }

    void print() const;
    void printDetailed() const;
};

} // namespace tms

#endif // TMS_STATION_H
