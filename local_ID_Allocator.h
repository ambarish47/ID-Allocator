/***************************************************************************
 * Name    : local_ID_Allocator.h                                          *
 *                                                                         *
 * Desc    : allocates local ids , dynamically.                            *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#ifndef __LOCAL_ID_ALLOCATOR__
#define __LOCAL_ID_ALLOCATOR__
#include "fixedSizeRangeSet.h"

#define LIDA_BLOCK (std::string("LIDA::")+ __FUNCTION__ + "(): ")

class Local_ID_Allocator
{
    public :
        const int mSlotNo;
        Local_ID_Allocator(int slot) : mSlotNo(slot) {
            if(slot >= 0)
            {
                mMap_ConsecutiveIDs.insert( std::pair<int , FixedSizeRangeSet>(16000, FixedSizeRangeSet(1 , 16000 , mSlotNo)) );
            }
        }

        virtual ~Local_ID_Allocator()  { }

        bool getLocalIDs( int& start , const int total_ids , std::string& errMsg);
        bool saveLocalID( const int id , std::string& errMsg);
        bool fillCompleteRangeSet(std::map<int , RangeSet>& completeRangeSet , int minRangeSize , std::string& errMsg) const;
        bool deleteCommonlyAllocatedIDs(const int startingId , const int totalIDs , std::string& errMsg);

        void dumpIDstates(std::string & errMsg) const;
        friend class FixedSizeRangeSet;/*FixedSizeRangeSet accesses saveRange(...)*/

    protected :
        std::map < int , FixedSizeRangeSet > mMap_ConsecutiveIDs;
        /* int corresponds to number of consecutive Ids , a set stores ( set identifier ).*/

        int getFirstNonEmptySet(const int total_ids) const;

        bool isIDAlreadyPresent(const int id) const;
        bool allocateIDs(int & rangeStart , const int map_id , std::string& errMsg);
        bool deleteAllocatedIds(const int rangeSize , const int start , const int end , std::string& errMsg);
        bool get_RangeToBeSaved_And_delete_PredecessorRangeAndSuccessorRange(int& rangeStart , int& rangeEnd , const int id , std::string& errMsg );
        bool saveRange(const int rangeStart , const int rangeEnd , std::string& errMsg);

};


#endif
