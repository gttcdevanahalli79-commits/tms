#ifndef TMS_TRAIN_H
#define TMS_TRAIN_H

#include "Common.h"
#include <string>
#include <vector>
#include <iostream>

namespace tms {

class Train {
private:
    int m_trainId;
    std::string m_trainNumber; // 5-digit train number (e.g., "12951")
    std::string m_trainName;
    TrainType m_type;
    int m_sourceStationId;
    int m_destStationId;
    int m_routeId;
    double m_baseFarePerKm;
    bool m_isActive;

public:
    Train(int id, std::string number, std::string name, TrainType type,
          int srcId, int dstId, int routeId, double baseFare = 0.50, bool active = true);

    int getTrainId() const { return m_trainId; }
    const std::string& getTrainNumber() const { return m_trainNumber; }
    const std::string& getTrainName() const { return m_trainName; }
    TrainType getType() const { return m_type; }
    int getSourceStationId() const { return m_sourceStationId; }
    int getDestStationId() const { return m_destStationId; }
    int getRouteId() const { return m_routeId; }
    double getBaseFarePerKm() const { return m_baseFarePerKm; }
    bool isActive() const { return m_isActive; }

    void setTrainName(const std::string& name) { m_trainName = name; }
    void setType(TrainType type) { m_type = type; }
    void setBaseFarePerKm(double fare) { m_baseFarePerKm = fare; }
    void setActive(bool active) { m_isActive = active; }

    void print() const;
};

} // namespace tms

#endif // TMS_TRAIN_H
