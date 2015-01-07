id_allocator.lo :
	g++ -g id_Allocator.cpp local_ID_Allocator.cpp hwUtils.cpp rangeSet.cpp fixedSizeRangeSet.cpp

all :
	ld id_Allocator.lo -o a.out
