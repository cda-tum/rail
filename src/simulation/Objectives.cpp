#include "simulation/Objectives.hpp"

double cda_rail::sim::combined_objective(const TrainTrajectorySet& traj_set) {
  /**
   * Combined objectives
   */
  return collision_penalty(traj_set) + destination_penalty(traj_set) +
         stop_penalty(traj_set);
}

double cda_rail::sim::collision_penalty(const TrainTrajectorySet& traj_set) {
  /**
   * Check all trains for violation in minimum distances
   * Train position is assumed to be the center of the train
   *
   * @param traj_set Set of train trajectories
   * @param dist_penalty_fct Penalty invoked when distance is smaller than
   * safety_distance Distance argument is normalized (0-1)
   * @param safety_distance  Distance between trains below which penalties are
   * applied
   * @return Normalized penalty score from 0 to 1, lower is better
   */

  constexpr double safety_distance = 100;

  const SimulationInstance& instance   = traj_set.get_instance();
  const TrainList&          train_list = instance.timetable.get_train_list();
  double                    score      = 0;

  for (auto train1 = train_list.begin(); train1 != train_list.end(); train1++) {
    const TrainTrajectory& traj1       = traj_set.get_traj((*train1).name);
    size_t                 first_step1 = traj1.get_first_timestep();
    size_t                 last_step1  = traj1.get_last_timestep();

    for (auto train2 = train1 + 1; train2 != train_list.end(); train2++) {
      const TrainTrajectory& traj2       = traj_set.get_traj((*train2).name);
      size_t                 first_step2 = traj2.get_first_timestep();
      size_t                 last_step2  = traj2.get_last_timestep();

      if (last_step1 < first_step2 || last_step2 < first_step1)
        continue;

      double required_dist =
          0.5 * (*train1).length + 0.5 * (*train2).length + safety_distance;

      if (2 * required_dist < (*train1).max_speed + (*train2).max_speed)
        throw std::logic_error("Time resolution too low.");

      for (size_t timestep = std::max(first_step1, first_step2);
           timestep <= std::min(last_step1, last_step2);) {
        double dist =
            traj_set.train_distance((*train1).name, (*train2).name, timestep)
                .value();

        if (dist >= required_dist) {
          double max_approach_speed = (*train1).max_speed + (*train2).max_speed;
          double min_time_to_collision =
              std::max((dist - required_dist), 0.0) / max_approach_speed;
          size_t guaranteed_safe_time = std::floor(min_time_to_collision);
          timestep = timestep + std::max(guaranteed_safe_time, (size_t)1);
        } else {
          double penalty = 1 - (dist / required_dist);
          score          = score + penalty;
          timestep++;
        }
      }
    }
  }

  size_t n_pairs = (train_list.size() * (train_list.size() - 1)) / 2;
  return score / n_pairs;
}

double cda_rail::sim::destination_penalty(const TrainTrajectorySet& traj_set) {
  /**
   * Penalize all trains for the distance from their scheduled exit at their
   * final position Train position is assumed to be the center of the train
   *
   * @param traj_set Set of train trajectories
   * @return Normalized penalty score from 0 to 1, lower is better
   */

  const SimulationInstance& instance   = traj_set.get_instance();
  const TrainList&          train_list = instance.timetable.get_train_list();
  double                    score      = 0;

  for (auto train = train_list.begin(); train != train_list.end(); train++) {
    size_t dest_vertex =
        instance.timetable.get_schedule((*train).name).get_exit();
    size_t final_timestep =
        traj_set.get_traj((*train).name).get_last_timestep();
    double max_dist =
        *std::max_element(instance.shortest_paths.at(dest_vertex).begin(),
                          instance.shortest_paths.at(dest_vertex).end());
    ;
    double dist =
        traj_set
            .train_vertex_distance((*train).name, dest_vertex, final_timestep)
            .value();
    score = score + dist / max_dist;
  }
  return score / train_list.size();
}

double cda_rail::sim::stop_penalty(const TrainTrajectorySet& traj_set) {
  /**
   * Penalize trains not visiting their scheduled stop
   *
   * @param traj_set Set of train trajectories
   * @return Normalized penalty score from 0 to 1, lower is better
   */

  const SimulationInstance& instance   = traj_set.get_instance();
  const TrainList&          train_list = instance.timetable.get_train_list();
  size_t                    n_all_visited_stops   = 0;
  size_t                    n_all_scheduled_stops = 0;
  for (auto train = train_list.begin(); train != train_list.end(); train++) {
    size_t n_scheduled_stops =
        instance.timetable.get_schedule((*train).name).get_stops().size();
    size_t n_visited_stops =
        traj_set.get_traj((*train).name).get_visited_stop_amount();
    if (n_visited_stops > n_scheduled_stops)
      throw std::logic_error("Visited more stops than scheduled.");
    n_all_scheduled_stops += n_scheduled_stops;
    n_all_visited_stops += n_visited_stops;
  }

  return (n_all_scheduled_stops - n_all_visited_stops) / n_all_scheduled_stops;
}
