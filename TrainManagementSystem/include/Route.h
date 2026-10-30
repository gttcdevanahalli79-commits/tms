#ifndef TMS_ROUTE_H
#define TMS_ROUTE_H

#include "Station.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace tms {

struct RouteStop {
    int stopOrder;
    int stationId;
    std::string stationCode;
    std::string stationName;
    double distanceFromStartKm;
    int haltMinutes;
};

class Route {
private:
    int m_routeId;
    std::string m_routeName;
    int m_sourceStationId;
    int m_destStationId;
    double m_totalDistanceKm;
    std::vector<RouteStop> m_stops;

public:
    Route(int id, std::string name, int srcId, int dstId, double totalDist = 0.0);

    int getRouteId() const { return m_routeId; }
    const std::string& getRouteName() const { return m_routeName; }
    int getSourceStationId() const { return m_sourceStationId; }
    int getDestStationId() const { return m_destStationId; }
    double getTotalDistanceKm() const { return m_totalDistanceKm; }
    const std::vector<RouteStop>& getStops() const { return m_stops; }

    void addStop(const RouteStop& stop);
    void setStops(const std::vector<RouteStop>& stops);
    double getDistanceBetweenStations(int startStationId, int endStationId) const;
    bool containsSegment(int fromStationId, int toStationId) const;

    void print() const;
};

} // namespace tms

#endif // TMS_ROUTE_H
