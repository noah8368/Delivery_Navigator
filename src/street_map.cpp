//  Authors: Noah Himed
//  Date:    19 March 2020
//  Summary: Defines a data structure that stores map data into a graph
//           structure gets adjoining streets for any given street.

#include "provided.h"
#include "expandable_hash_map.h"

#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <cctype>
#include <cstring>
#include <cstdlib>

using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return static_cast<unsigned int>(std::hash<string>()(g.latitudeText + g.longitudeText));
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool Load(string map_data_path);
    // given a GeoCoord object which defines a point on a street, supply a vector of connecting
    // street segments
    bool GetSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    // private auxillary function, adds segment to map
    void AppendSegToMap(GeoCoord coord, StreetSegment seg);
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_coord_seg_map;
};

StreetMapImpl::StreetMapImpl()
{}

StreetMapImpl::~StreetMapImpl()
{}

bool StreetMapImpl::Load(string map_data_path)
{
    ifstream map_data_file(map_data_path);
    if(!map_data_file)
        return false;
    
    int line_num = 1, num_sig_figs = 7, num_segs = 0;
    StreetSegment forward_seg, backward_seg;
    string input_ln, start_lattitude, start_longitude, end_lattitude, end_longitude;
    string* seg_end_points[4] = {&start_lattitude, &start_longitude, &end_lattitude, &end_longitude};
    char next_char;
    
    // iterate through supplied map data file line by line,
    // loading in information for line segments defining streets
    while(getline(map_data_file, input_ln))
    {
        istringstream map_data(input_ln);
        map_data.setf(ios::fixed);
        map_data.precision(num_sig_figs);
        switch (line_num)
        {
            case 1:
                forward_seg.name = input_ln;
                backward_seg.name = input_ln;
                break;
                
            case 2:
                map_data >> num_segs;
                break;
            default:
            {
                // modify start_lattitude, start_longitude, end_lattitude, end_longitude
                // strings for each defined street segment
                for (auto& seg_point_str : seg_end_points)
                {
                    *seg_point_str = "";
                    while(map_data.get(next_char) &&
                      (isdigit(next_char) || next_char == '.' || next_char == '-'))
                    *seg_point_str += next_char;
                }
                
                GeoCoord start_coord(start_lattitude, start_longitude), end_coord(end_lattitude, end_longitude);
                
                forward_seg.start = backward_seg.end = start_coord;
                forward_seg.end = backward_seg.start = end_coord;
                
                // appends same line segment associated with both end points
                // in street map
                AppendSegToMap(start_coord, forward_seg);
                AppendSegToMap(end_coord, backward_seg);
            }
        }
        
        line_num++;
        // reset line number when we come to the end of a street
        line_num = (num_segs != 0 && line_num - 2 == num_segs) ? 0 : line_num;
    }
    
    map_data_file.close();
    return true;
}

bool StreetMapImpl::GetSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment> *segVecPtr = m_coord_seg_map.Find(gc);
    if(segVecPtr != nullptr)
        segs = *segVecPtr;
    return (bool(segVecPtr));  
}

void StreetMapImpl::AppendSegToMap(GeoCoord coord, StreetSegment seg)
{
    vector<StreetSegment>* mappedSegs = m_coord_seg_map.Find(coord);
    // if the coordinate is already in the map, add the adjoining street
    if(mappedSegs != nullptr)
        mappedSegs -> push_back(seg);
    else
    {
        vector<StreetSegment> new_segs;
        new_segs.push_back(seg);
        m_coord_seg_map.Associate(seg.start, new_segs);
    }
}

// Functions added by Professors Nachenburg and Smallberg for grading purposes

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string map_data_path)
{
    return m_impl->Load(map_data_path);
}

bool StreetMap::GetSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->GetSegmentsThatStartWith(gc, segs);
}
