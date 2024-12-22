#include "simulation/RoutingSolution.hpp"

cda_rail::RoutingSolution::RoutingSolution(ulong n_v_target_vars,
                                           ulong n_switch_vars,
                                           ulong n_timesteps,
                                           const cda_rail::Train& train,
                                           std::ranlux24_base&    rng_engine) {
  std::uniform_int_distribution<ulong>   uniform_int(0, n_timesteps - 1);
  std::uniform_real_distribution<double> uniform(0, 1);
  std::uniform_real_distribution<double> uniform_train_speed(-train.max_speed,
                                                             train.max_speed);

  switch_directions.reserve(n_switch_vars);
  for (ulong i = 1; i <= n_switch_vars; i++) {
    switch_directions.push_back(uniform(rng_engine));
  }

  while (v_targets.targets.size() < n_v_target_vars) {
    v_targets.targets.insert(
        {uniform_int(rng_engine), uniform_train_speed(rng_engine)});
  }
}
