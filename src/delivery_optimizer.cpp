//  Authors: Noah Himed
//  Date:    19 March 2020
//  Summary: Utilizes the Simulated Annealing algorithm to
//           efficiently find a highly optimized solution to
//           the Traveling Salesman problem of ordering deliveries.

#include "provided.h"

#include <vector>
#include <iterator>
#include <ctime>
#include <cmath>
#include <random>

using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    // takes in a vector of objects detailing food delivery
    // locations, and optimizes the order of those deliveries
    // to reduce distance travelled by the driver
    void OptimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& old_crow_dist,
        double& new_crow_dist) const;
private:
    const StreetMap *m_smPtr;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
    m_smPtr = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{}

void DeliveryOptimizerImpl::OptimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& old_crow_dist,
    double& new_crow_dist) const
{
    // "crow distance" describes the distance between a set of locations
    // measured along straight lines, point to point
    new_crow_dist = 0;
    old_crow_dist = 0;
    
    vector<DeliveryRequest> delivery_path;
    // add depot to delivery path
    delivery_path.emplace_back("", depot);
    for(DeliveryRequest request: deliveries)
        delivery_path.push_back(request);
    delivery_path.emplace_back("", depot);
    
    // compute old_crow_dist
    for(auto delivery_it = delivery_path.begin(); delivery_it != prev(delivery_path.end()); delivery_it++)
        old_crow_dist += distanceEarthMiles(delivery_it->location, next(delivery_it)->location);
    
    // Ultilize Simmulated Annealing algorithm to optimize delivery order
    // Here, the idea is that random changes are made more frequently in the
    // beginning when the "temperature" (a measure of the time since the algorithm
    // began) is high at start, and less frequently when temperature is low at the end
    if(deliveries.size() > 1)
    {
        // seed random number generator
        srand(unsigned(time(0)));
        int max_iterations = 100;
        int num_stops = int(delivery_path.size()), num_middle_stops = num_stops-2;
        int max_paths_temp = max_iterations*num_stops, max_paths_before_cont = max_iterations*num_stops, num_passes;
        double new_path_len, curr_path_len = old_crow_dist, cost_diff, temperature = 0.5;
        
        int num_start_coords, num_end_coords;
        vector<DeliveryRequest> temp_path;
        
        // loop through max_iterations number of period with same temperature
        for(int i = 0; i < max_iterations; i++)
        {
            num_passes = 0;
            for(int j = 0; j < max_paths_temp; j++)
            {
                // randomly select a section
                do
                {
                    num_start_coords = (rand() % num_middle_stops) + 1;
                    num_end_coords = (rand() % num_middle_stops) + 1;
                    if(num_end_coords == num_middle_stops+1)
                        num_end_coords--;
                    if(num_start_coords > num_end_coords)
                    {
                        int tempCoord = num_end_coords;
                        num_end_coords = num_start_coords;
                        num_start_coords = tempCoord;
                    }
                }
                // number of stops not in section should be >= ~20% number of stops
                while((num_start_coords == num_end_coords)
                      || (num_stops-(num_end_coords-num_start_coords)-1) < 0.2*num_stops);
                
                // flip the section half the time
                if(static_cast<int>(rand() % 2) != 0)
                {
                    for(int k = 0; k < num_start_coords; k++)
                        temp_path.push_back(delivery_path[k]);
                    for(int k = num_end_coords; k >= num_start_coords; k--)
                        temp_path.push_back(delivery_path[k]);
                    for(int k = num_end_coords+1; k < num_stops; k++)
                        temp_path.push_back(delivery_path[k]);
                }
                // otherwise, we move the section to somewhere else in the path
                else
                {
                    temp_path = delivery_path;
                    auto delivery_it = temp_path.begin()+num_start_coords;
                    for(int k = 0; k < (num_end_coords-num_start_coords)+1; k++)
                        delivery_it = temp_path.erase(delivery_it);
                    int new_position = rand() % (num_stops-(num_end_coords-num_start_coords)-2) + 1;
                    auto insert_it = temp_path.begin()+new_position;
                    for(int k = 0; k < (num_end_coords-num_start_coords)+1; k++)
                        temp_path.insert(insert_it+k, *(delivery_path.begin()+num_start_coords+k));
                }
                
                // get length of new path
                new_path_len = 0;
                for(auto new_delivery_it = temp_path.begin(); new_delivery_it != prev(temp_path.end()); new_delivery_it++)
                    new_path_len += distanceEarthMiles(new_delivery_it->location, next(new_delivery_it)->location);
                
                cost_diff = new_path_len - curr_path_len;
                // if new path is shorter, use it
                if(cost_diff < 0.0)
                {
                    delivery_path = temp_path;
                    curr_path_len = new_path_len;
                    num_passes++;
                }
                // if the new path is longer, use it sometimes under random chance
                else if(cost_diff > 0.0)
                {
                    // seed random number generator
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_real_distribution<> random_num(0.0, 1.0);
                    // the probability this segment is used decreased with the temperature
                    if(double(random_num(gen)) < double(exp(-cost_diff/temperature)))
                    {
                        delivery_path = temp_path;
                        curr_path_len = new_path_len;
                        num_passes++;
                    }
                }
                temp_path.clear();
                if(num_passes >= max_paths_before_cont)
                    break;
            }
            temperature *= 0.9;
        }
    }
    
    // compute new crow distance
    for(auto delivery_it = delivery_path.begin(); delivery_it != prev(delivery_path.end()); delivery_it++)
        new_crow_dist += distanceEarthMiles(delivery_it->location, next(delivery_it)->location);
    
    // remove "delivery orders" standing in for depot at beggning and end
    delivery_path.erase(delivery_path.begin());
    delivery_path.pop_back();
    
    deliveries = delivery_path;
}

// Functions added by Professors Nachenburg and Smallberg for grading purposes

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::OptimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& old_crow_dist,
        double& new_crow_dist) const
{
    return m_impl->OptimizeDeliveryOrder(depot, deliveries, old_crow_dist, new_crow_dist);
}
