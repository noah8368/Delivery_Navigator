//  Authors: Noah Himed
//  Date:    19 March 2020
//  Summary: Uses other defined classes to generate a
//           set of turn-by-turn directions to complete all
//           deliveries with minimized distance travelled.

#include "provided.h"
#include "expandable_hash_map.h"

#include <vector>
#include <list>
#include <iostream>
#include <iterator>

using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult GenerateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& total_dist_travelled) const;
private:
    DeliveryResult AddDelivery(DeliveryResult& delivery_status, list<StreetSegment>& path_buffer,
                               list<StreetSegment>& total_path, double& new_distance,
                               PointToPointRouter& path, const GeoCoord& start, const GeoCoord& end,
                               list<StreetSegment>& route, double& total_dist_travelled) const;
    const StreetMap *m_sm_ptr;
};

string getProceedDirection(StreetSegment seg);
double getSegDist(StreetSegment seg);

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_sm_ptr = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{}

DeliveryResult DeliveryPlannerImpl::GenerateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& total_dist_travelled) const
{
    // use delivery optimizer to reorder deliveries
    DeliveryOptimizer optimization_engine(m_sm_ptr);
    double old_crow_dist, new_crow_dist;
    vector<DeliveryRequest> optimized_deliveries = deliveries;
    optimization_engine.OptimizeDeliveryOrder(depot, optimized_deliveries, old_crow_dist, new_crow_dist);
    
    DeliveryResult delivery_status;
    total_dist_travelled = 0;
    double new_distance = 0;
    list<StreetSegment> total_path, path_buffer;
    PointToPointRouter path(m_sm_ptr);
        
    // get the path from depot to first locaton
    if (AddDelivery(delivery_status, path_buffer, total_path, new_distance, path, depot, optimized_deliveries.front().location, path_buffer, total_dist_travelled) != DELIVERY_SUCCESS)
        return delivery_status;
        
    // generate a route between all delivery locations
    for(auto delivery_it = optimized_deliveries.begin(); delivery_it != prev(optimized_deliveries.end()); delivery_it++)
        if(AddDelivery(delivery_status, path_buffer, total_path, new_distance, path, delivery_it->location,
           next(delivery_it)->location, path_buffer, total_dist_travelled) != DELIVERY_SUCCESS)
            return delivery_status;
        
    // get the path from the last location to the depot
    if (AddDelivery(delivery_status, path_buffer, total_path, new_distance, path, optimized_deliveries.back().location, depot, path_buffer, total_dist_travelled) != DELIVERY_SUCCESS)
        return delivery_status;
        
    // generate commands
    auto delivery_tracker = optimized_deliveries.begin();
    DeliveryCommand next_command;
    double angle;
    
    // if there is a delivery at the depot
    if(delivery_tracker->location == total_path.front().start)
    {
        next_command.InitAsDeliverCommand(delivery_tracker -> item);
        commands.push_back(next_command);
        delivery_tracker++;
    }
    
    // put in first proceed command
    next_command.InitAsProceedCommand(getProceedDirection(total_path.front()),
                                     total_path.front().name,
                                     getSegDist(total_path.front()));
    commands.push_back(next_command);
    
    // fill in rest of commands until depot at end
    for(auto current_seg = next(total_path.begin()); current_seg != total_path.end();
        current_seg++)
    {
        // first, check if we need to deliver to the current location
        if(delivery_tracker->location == current_seg->start)
        {
            next_command.InitAsDeliverCommand(delivery_tracker->item);
            commands.push_back(next_command);
            delivery_tracker++;
            // next command will be a Proceed
            next_command.InitAsProceedCommand(getProceedDirection(*current_seg),
                                             current_seg->name,
                                             getSegDist(*current_seg));
            commands.push_back(next_command);
            continue;
        }
        
        auto prev_line = prev(current_seg);
        // above line modifies current_seg, so we need to compensate
        // by using next() on the next line
        auto current_line = next(prev_line);
        angle = angleBetween2Lines(*prev_line, *current_line);
        
        // check if it's a proceeed case first
        if(angle < 1.0 || angle > 359.0)
        {
            // first, check if last command was proceed of same name
            if(commands.back().Description()[0] == 'P' && commands.back().StreetName() == current_seg->name)
                commands.back().IncreaseDistance(getSegDist(*current_seg));
            else
            {
                next_command.InitAsProceedCommand(getProceedDirection(*current_seg), current_seg->name,
                                                  getSegDist(*current_seg));
                commands.push_back(next_command);
            }
        }
        // must be a turn case
        else
        {
            // if it's a turn, that's our first action on a segment
            if(angle >= 1.0 && angle < 180.0)
                next_command.InitAsTurnCommand("left", current_seg->name);
            else if(angle >= 180.0 && angle <= 359.0)
                next_command.InitAsTurnCommand("right", current_seg->name);
            
            if(current_seg->name != prev_line->name)
            {
                commands.push_back(next_command);
                // we must use Proceed to get to the next segment
                next_command.InitAsProceedCommand(getProceedDirection(*current_seg), current_seg->name,
                                                  getSegDist(*current_seg));
                commands.push_back(next_command);
            }
            else
                commands.back().IncreaseDistance(getSegDist(*current_seg));
        }
    }
    // check if there's a delivery at the depot at the very end
    if(delivery_tracker != optimized_deliveries.end())
        next_command.InitAsDeliverCommand(delivery_tracker->item);
        
    return DELIVERY_SUCCESS;
}

DeliveryResult DeliveryPlannerImpl::AddDelivery(DeliveryResult& delivery_status, list<StreetSegment>& path_buffer,
                                                list<StreetSegment>& total_path, double& new_distance,
                                                PointToPointRouter& path, const GeoCoord& start, const GeoCoord& end,
                                                list<StreetSegment>& route, double& total_dist_travelled) const
{
    delivery_status = path.GeneratePointToPointRoute(start, end, route, total_dist_travelled);
    if(delivery_status != DELIVERY_SUCCESS)
        return delivery_status;
    else
    {
        total_path.splice(total_path.end(), path_buffer);
        total_dist_travelled += new_distance;
        return DELIVERY_SUCCESS;
    }
}

// assign direction to angle
string getProceedDirection(StreetSegment seg)
{
    double angle = angleOfLine(seg);
    if(angle >= 0 && angle < 22.5)
        return "east";
    else if(angle >= 22.5 && angle < 67.5)
        return "northeast";
    else if(angle >= 67.5 && angle < 112.5)
        return "north";
    else if(angle >= 112.5 && angle < 157.5)
        return "northwest";
    else if(angle >= 157.5 && angle < 202.5)
        return "west";
    else if(angle >= 202.5 && angle < 247.5)
        return "southwest";
    else if(angle >= 247.5 && angle < 292.5)
        return "south";
    else if(angle >= 292.5 && angle < 337.5)
        return "southeast";
    else if(angle >= 337.5)
        return "east";
    else
        return "INVALID";
}

double getSegDist(StreetSegment seg)
{
    return distanceEarthMiles(seg.start, seg.end);
}

// Functions added by Professors Nachenburg and Smallberg for grading purposes

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::GenerateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& total_dist_travelled) const
{
    return m_impl->GenerateDeliveryPlan(depot, deliveries, commands, total_dist_travelled);
}
