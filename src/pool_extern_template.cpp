// Parachute
#include <parachute/pool.hpp>

#ifndef PARACHUTE_COMPILED
#error "expecting PARACHUTE_COMPILED to be defined"
#endif // PARACHUTE_COMPILED

namespace para
{

template class pool_base<work_group_static<1>, work_queue_lifo<>, work_control_default>;
template class pool_base<work_group_static<1>, work_queue_lifo<>, work_control_strict>;
template class pool_base<work_group_dynamic, work_queue_lifo<>, work_control_default>;
template class pool_base<work_group_dynamic, work_queue_lifo<>, work_control_strict>;

}  // namespace para