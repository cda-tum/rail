#pragma once
#include "datastructure/GeneralTimetable.hpp"
#include "datastructure/RailwayNetwork.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cda_rail {
class ScheduledStop : public GeneralScheduledStop {
  /**
   * A scheduled stop with fixed times
   */

public:
  [[nodiscard]] static int time_type() {
    // return the type of the desired time type
    return int();
  }

  [[nodiscard]] int arrival() const {
    return GeneralScheduledStop::get_begin_range().first;
  }
  [[nodiscard]] int departure() const {
    return GeneralScheduledStop::get_end_range().first;
  }

  // Constructor
  ScheduledStop(int begin, int end, std::string station)
      : GeneralScheduledStop({begin, begin}, {end, end}, end - begin,
                             std::move(station)) {}
};

class Schedule : public GeneralSchedule<ScheduledStop> {
  /**
   * Specific Schedule object
   */

public:
  [[nodiscard]] int get_t_0() const { return get_t_0_range().first; }
  [[nodiscard]] int get_t_n() const { return get_t_n_range().first; }

  void set_t_0(int t_0) { set_t_0_range({t_0, t_0}); }
  void set_t_n(int t_n) { set_t_n_range({t_n, t_n}); }

  // Constructor
  // NOLINTNEXTLINE(readability-redundant-member-init)
  Schedule() : GeneralSchedule() {}
  Schedule(int t_0, double v_0, size_t entry, int t_n, double v_n, size_t exit,
           std::vector<ScheduledStop> stops = {})
      : GeneralSchedule({t_0, t_0}, v_0, entry, {t_n, t_n}, v_n, exit,
                        std::move(stops)) {}
};

class Timetable : public GeneralTimetable<Schedule> {
  /**
   * Timetable class
   */
public:
  // Constructors
  Timetable() = default;
  Timetable(const std::filesystem::path& p, const Network& network)
      : GeneralTimetable(p, network){};
  Timetable(const std::string& path, const Network& network)
      : Timetable(std::filesystem::path(path), network){};
  Timetable(const char* path, const Network& network)
      : Timetable(std::filesystem::path(path), network){};

  // Rule of 5
  Timetable(const Timetable& other)                = default;
  Timetable(Timetable&& other) noexcept            = default;
  Timetable& operator=(const Timetable& other)     = default;
  Timetable& operator=(Timetable&& other) noexcept = default;
  virtual ~Timetable()                             = default;

  using GeneralTimetable::add_stop;
  void add_stop(size_t train_index, const std::string& station_name, int begin,
                int end) {
    GeneralTimetable::add_stop(train_index, station_name, true, begin, end);
  };
  void add_stop(const std::string& train_name, const std::string& station_name,
                int begin, int end) {
    GeneralTimetable::add_stop(train_list.get_train_index(train_name),
                               station_name, true, begin, end);
  };

  [[nodiscard]] int                 max_t() const;
  [[nodiscard]] std::pair<int, int> time_interval(size_t train_index) const;
  [[nodiscard]] std::pair<int, int>
  time_interval(const std::string& train_name) const {
    return time_interval(train_list.get_train_index(train_name));
  };
  [[nodiscard]] std::pair<size_t, size_t>
  time_index_interval(size_t train_index, int dt,
                      bool tn_inclusive = true) const;
  [[nodiscard]] std::pair<size_t, size_t>
  time_index_interval(const std::string& train_name, int dt,
                      bool tn_inclusive = true) const {
    return time_index_interval(train_list.get_train_index(train_name), dt,
                               tn_inclusive);
  };

  [[nodiscard]] static Timetable import_timetable(const std::string& path,
                                                  const Network&     network) {
    return {path, network};
  };
  [[nodiscard]] static Timetable
  import_timetable(const std::filesystem::path& p, const Network& network) {
    return {p, network};
  };
  [[nodiscard]] static Timetable import_timetable(const char*    path,
                                                  const Network& network) {
    return {path, network};
  };
};
} // namespace cda_rail
