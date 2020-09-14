objects = delivery_optimizer.o delivery_planner.o main.o point_to_point_router.o street_map.o
exe_name = delivery_navigator

$(exe_name) : $(objects)
	g++ -o $(exe_name) $(objects)

delivery_optimizer.o : provided.h
	g++ -std=c++11 -c delivery_optimizer.cpp
delivery_planner.o : provided.h expandable_hash_map.h
	g++ -std=c++11 -c delivery_planner.cpp
main.o : provided.h
	g++ -std=c++11 -c main.cpp
point_to_point_router.o : provided.h expandable_hash_map.h
	g++ -std=c++11 -c point_to_point_router.cpp
street_map.o : provided.h expandable_hash_map.h
	g++ -std=c++11 -c street_map.cpp

.PHONY : clean
clean :
	-rm $(exe_name) $(objects)
