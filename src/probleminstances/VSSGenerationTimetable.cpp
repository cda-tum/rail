#include "probleminstances/VSSGenerationTimetable.hpp"
#include "datastructure/RailwayNetwork.hpp"
#include "Definitions.hpp"

void cda_rail::instances::VSSGenerationTimetable::export_instance(const std::filesystem::path &p) const {
    /**
     * Exports the instance to the given path, i.e.,
     * - the network into the folder "network"
     * - the timetable into the folder "timetable"
     * - the routes into the folder "routes"
     *
     * @param p the path to the folder where the instance should be exported
     */

    if (!cda_rail::is_directory_and_create(p)) {
        throw std::runtime_error("Could not create directory " + p.string());
    }
    network.export_network(p / "network");
    timetable.export_timetable(p / "timetable", network);
    routes.export_routes(p / "routes", network);
}

cda_rail::instances::VSSGenerationTimetable::VSSGenerationTimetable(const std::filesystem::path &p,
                                                                    bool every_train_must_have_route) {
    /**
     * Creates object and imports an instance from the given path, i.e.,
     * - the network from the folder "network"
     * - the timetable from the folder "timetable"
     * - the routes from the folder "routes"
     *
     * @param p the path to the folder where the instance should be imported from
     */

    this->network = cda_rail::Network::import_network(p / "network");
    this->timetable = cda_rail::Timetable::import_timetable(p / "timetable", this->network);
    this->routes = cda_rail::RouteMap::import_routes(p / "routes", this->network);
    if (!this->check_consistency(every_train_must_have_route)) {
        throw std::runtime_error("The imported instance is not consistent.");
    }
}

void cda_rail::instances::VSSGenerationTimetable::discretize(cda_rail::SeparationType separation_type) {
    /**
     * This method discretizes the network. It updates the timetable and the routes accordingly.
     *
     * @param separation_type the type of separation to be used
     */

    const auto new_edges = network.discretize(separation_type);
    timetable.update_after_discretization(new_edges);
    routes.update_after_discretization(new_edges);
}

std::vector<int>
cda_rail::instances::VSSGenerationTimetable::trains_in_section(const std::vector<int> &section) const {
    /**
     * Returns the trains that traverse the given section
     *
     * @param section: the section
     * @return the trains that traverse the given section
     */

    // Initialize the vector of trains
    std::vector<int> tr_in_sec;

    for (int i = 0; i < get_train_list().size(); ++i) {
        auto tr_name = get_train_list().get_train(i).name;
        auto tr_route = get_route(tr_name).get_edges();
        bool tr_in_sec_flag = false;
        for (int j = 0; j < section.size() && !tr_in_sec_flag; ++j) {
            for (int k = 0; k < tr_route.size() && !tr_in_sec_flag; ++k) {
                if (section[j] == tr_route[k]) {
                    tr_in_sec.emplace_back(i);
                    tr_in_sec_flag = true;
                }
            }
        }
    }

    return tr_in_sec;
}

std::vector<int> cda_rail::instances::VSSGenerationTimetable::trains_at_t(int t) const {
    /**
     * Returns a list of all trains present at time t.
     */

    if (t < 0) {
        throw std::invalid_argument("t must be non-negative.");
    }

    std::vector<int> trains;
    const auto& train_list = timetable.get_train_list();
    for (int i = 0; i < train_list.size(); ++i) {
        const auto& interval = timetable.time_interval(i);
        if (interval.first <= t && t <= interval.second) {
            trains.push_back(i);
        }
    }

    return trains;

}
