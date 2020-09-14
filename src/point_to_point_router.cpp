//  Authors: Noah Himed
//  Date:    19 March 2020
//  Summary: Utilizes the A* algorithm to efficiently create
//           an optimal route between two given coordinates

#include "provided.h"
#include "expandable_hash_map.h"

#include <list>
#include <queue>
#include <set>
#include <functional>

using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult GeneratePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& total_dist_travelled) const;
private:
    // defines a priority queue that sorts by value
    // instead of key as a minheap
    struct MinheapQueue
    {
        typedef pair<double, GeoCoord> pos;
        priority_queue<pos, vector<pos>, greater<pos>> PosQueue;
        
        inline bool IsEmpty() const
        {
            return PosQueue.empty();
        }
        
        inline void Insert(GeoCoord coord, double rank)
        {
            PosQueue.emplace(rank, coord);
        }
        
        inline void PopTop()
        {
            PosQueue.pop();
        }
        
        GeoCoord GetTopPos()
        {
            return PosQueue.top().second;
        }
        
    };
    
    // pairs GeoCoord objects with street names
    struct StreetPair
    {
        StreetPair(GeoCoord coord, string name=""): m_coord(coord), m_name(name) {}
        GeoCoord m_coord;
        string m_name;
    };
    
    const StreetMap *m_street_map_ptr;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_street_map_ptr = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{}

DeliveryResult PointToPointRouterImpl::GeneratePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& total_dist_travelled) const
{
    vector<StreetSegment> segs;
    route.clear();
    total_dist_travelled = 0.0;
    
    if(!(m_street_map_ptr->GetSegmentsThatStartWith(end, segs))
       || !(m_street_map_ptr->GetSegmentsThatStartWith(start, segs)))
        return BAD_COORD;
    else
    {
        // Utilization of the A* algorithm
        bool route_found = false;
        double next_move_cost, rank;
        vector<StreetSegment> next_moves;
        GeoCoord nextPos;
        MinheapQueue search_space;
        ExpandableHashMap<GeoCoord, StreetPair> prev_location;
        ExpandableHashMap<GeoCoord, double> move_cost;

        // set up the starting position
        search_space.Insert(start, 0);
        StreetPair start_pos_seg_pair(start);
        prev_location.Associate(start, start_pos_seg_pair);
        move_cost.Associate(start, 0);
        
        while(!search_space.IsEmpty())
        {
            GeoCoord currPos = search_space.GetTopPos();
            
            if(currPos == end)
            {
                route_found = true;
                break;
            }
            
            search_space.PopTop();
            
            m_street_map_ptr -> GetSegmentsThatStartWith(currPos, next_moves);
            for(StreetSegment nextSeg: next_moves)
            {
                nextPos = nextSeg.end;
                next_move_cost = *move_cost.Find(currPos) + distanceEarthMiles(currPos, nextPos);
                // add new position into search space
                if(!move_cost.Find(nextPos) || next_move_cost < *move_cost.Find(nextPos))
                {
                    move_cost.Associate(nextPos, next_move_cost);
                    // here, our heurisitic is just the distance to the end
                    rank = next_move_cost + distanceEarthMiles(currPos, end);
                    search_space.Insert(nextPos, rank);
                    StreetPair pos_seg_pair(currPos, nextSeg.name);
                    prev_location.Associate(nextPos, pos_seg_pair);
                }
            }
        }
        
        // save the instructions into the route list
        if(route_found)
        {
            GeoCoord endCoord = end;
            StreetPair start_pos_seg_pair = *prev_location.Find(end);
            string streetName = start_pos_seg_pair.m_name;
            // stop sequence indicated by "" as m_name
            while(start_pos_seg_pair.m_name != "")
            {
                StreetSegment routeSeg(start_pos_seg_pair.m_coord, endCoord, streetName);
                route.insert(route.begin(), routeSeg);
                total_dist_travelled += distanceEarthMiles(routeSeg.start, routeSeg.end);
                
                endCoord = start_pos_seg_pair.m_coord;
                start_pos_seg_pair = *prev_location.Find(endCoord);
                streetName = start_pos_seg_pair.m_name;
            }
        }
        return route_found ? DELIVERY_SUCCESS : NO_ROUTE;
    }
}

// Functions added by Professors Nachenburg and Smallberg for grading purposes

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::GeneratePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& total_dist_travelled) const
{
    return m_impl->GeneratePointToPointRoute(start, end, route, total_dist_travelled);
}

