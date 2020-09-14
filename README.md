# Delivery Navigation Software

### Noah Himed
### 14 September 2020

Given specifically formatted text files detailing map information and delivery requests, 
this program gives a set of turn-by-turn instructions optimized for minimum length. The 
program gives instructions for delivering items from a single depot to locations in the 
given map data. Both the A* and Simulated Annealing algorithms (as seen in `point_to_point_router.cpp` 
and `delivery_optimizer.cpp`, respectively) are implemented to generate a maximally efficient
result. Here, Simulated Annealing algorithm is used to find the ordering of delivery locations
that gives the shortest point to point distance, and the A* algorithm is used to find the shortest
possible street paths between each delivery location.

The supplied text files `mapdata.txt` and `deliveries.txt` provide map data for the Westwood 
neighborhood and delivery locations in this area, respectively.

## Using and Building the Software

Simply use the provided Makefile to create the `delivery_navigator` executable file, which
may be used with appropriately formatted map information and delivery request files like so
in BASH:

	./delivery_navigator /path/to/map/data/mapdata.txt /path/to/delivery/requests/deliveries.txt

This will generate a list of directions to follow to visit all supplied delivery locations. Using the 
example files supplied in this repository under `files`, the following printout is generated:

	Generating route...

	Starting at the depot...
	Proceed north on Broxton Avenue for 0.08 miles
	Turn right on Le Conte Avenue
	Proceed east on Le Conte Avenue for 0.10 miles
	Turn left on Westwood Plaza
	Proceed northwest on Westwood Plaza for 0.35 miles
	DELIVER Math Textbooks (Eng IV)
	Proceed west on Strathmore Place for 0.20 miles
	Turn right on Charles E Young Drive West
	Proceed north on Charles E Young Drive West for 0.19 miles
	Turn left on De Neve Drive
	Proceed west on De Neve Drive for 0.06 miles
	DELIVER B-Plate Salmon (Sproul Landing)
	Proceed east on De Neve Drive for 0.06 miles
	Turn right on Charles E Young Drive West
	Proceed south on Charles E Young Drive West for 0.19 miles
	Turn right on Strathmore Place
	Proceed west on Strathmore Place for 0.03 miles
	DELIVER JBL Speakers (Beta Theta Pi)
	Proceed southeast on Gayley Avenue for 0.37 miles
	Turn left on Le Conte Avenue
	Proceed east on Le Conte Avenue for 0.07 miles
	Turn right on Broxton Avenue
	Proceed south on Broxton Avenue for 0.08 miles
	You are back at the depot and your deliveries are done!
	0.52 miles travelled for all deliveries.

## Formatting Map Data and Delivery Requests

Both supplied text files must follow the format of the example files EXACTLY, 
without any trailing whitespace or additional lines. For the `mapdata.txt` file, 
this means putting the name of each street its own line, with the number of line
segments making up that street on the next line, followed by the beggining and end
coordinates for each segment on consequtive lines as shown. 

	[street name 1]
	[number of line segments in street]
	[line segment 1 start point longitude and lattitude] [line segment 1 end point longitude and lattitude]
	...
        [last line segment start point longitude and lattitude] [last line segment end point longitude and lattitude]
	[sreet name 2]
	...

For the `deliveries.txt` file, the coordinates for the depot must be on the first line, following by each delivery location
on a separate line. For each delivery location, the coordinates of the location must come first on the line, following by a 
colon, the name of the item to be delivered, a space, and then the name of the delivery location.

	[depot longitude and lattitude]
	[delivery location 1 longitude and lattitude]:[Item 1] (delivery location 1 name)
	...

## Acknowedgment of CS32 Coursework

This program was written as a solution to Project 4 for the class COM SCI 32 at UCLA. Any function ending with `Impl` 
was created by the instructors (Carey Nachenburg and David Smallberg) for the purposes of grading. 

This class is notorious for students cheating by copying their projects off of GitHub. To be perfectly clear:

**STUDENTS ENROLLED IN CS32 OR ANY OTHER PROGRAMMING COURSE WHO USE TURN IN THIS CODE AS THEIR OWN FOR CREDIT ARE
DOING SO WITHOUT MY PERMISSION. I DO NOT CONDONE ANY USE OF THIS SOFTWARE THAT MAY CONSTITUTE PLAGARISM.** 
