#ifndef TMS_TRAIN_MANAGER_H
#define TMS_TRAIN_MANAGER_H

#include "Train.h"
#include "Station.h"
#include "Route.h"
#include "Schedule.h"
#include "Coach.h"
#include <vector>
#include <memory>
#include <map>

namespace tms {

class TrainManager {
public:
    TrainManager();

    // Train management
    bool addTrain(const Train& train);
    bool updateTrain(const Train& train);
    bool deleteTrain(int trainId);
    std::shared_ptr<Train> getTrainById(int trainId);
    std::shared_ptr<Train> getTrainByNumber(const std::string& trainNumber);
    std::vector<Train> getAllTrains();

    // Station management
    bool addStation(const Station& station);
    bool updateStation(const Station& station);
    bool deleteStation(int stationId);
    std::shared_ptr<Station> getStationByCode(const std::string& code);
    std::vector<Station> getAllStations();

    // Route management
    bool addRoute(const Route& route);
    std::shared_ptr<Route> getRouteById(int routeId);

    // Schedule management
    bool addSchedule(const Schedule& schedule);
    std::vector<Schedule> getSchedulesByTrainId(int trainId);

    // Search operations
    struct SearchResult {
        Train train;
        Schedule schedule;
        double distanceKm;
        int availableSeats;
        int64_t farePaise;
    };

    std::vector<SearchResult> searchTrains(const std::string& srcCode, const std::string& dstCode,
                                          const std::string& journeyDate, CoachClass cClass);
};

} // namespace tms

#endif // TMS_TRAIN_MANAGER_H
