/***************************************************************************
 * Name    : rangeSet.h                                                    *
 *                                                                         *
 * Desc    : api to store ids in form of ranges.                           *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#ifndef __RANGE_SET__
#define __RANGE_SET__

#include"hwUtils.h"
#define RANGE_SET_BLOCK (std::string("RangeSet::")+ __FUNCTION__ + "(): ")

class RangeSet
{
    public :
        RangeSet(int sn) : mSlotNo(sn) {}
        enum ID_TYPE { SINGLE_ID = 0 , STARTING_ID = 1 , ENDING_ID = 2 };
        static const unsigned int MIN_ID = 1 , MAX_ID = 16000;

        bool isPredecessorPresent(const int id) const;
        bool isIDPresent         (const int id) const;
        bool isSuccessorPresent  (const int id) const;
        bool fillCompleteRangeSet(RangeSet& completeRangeSet , std::string& errMsg) const;
        virtual bool dumpIDstates(std::string& errMsg , const int rangeSize = 0) const;
        bool getPredecessorRangeStart(const int id , int& rangeStart , std::string& errMsg) const;
        bool getSuccessorRangeEnd(const int id , int& rangeStart , std::string& errMsg) const;
        virtual bool deleteRange(const int start , const int end , std::string& errMsg);
        virtual bool saveRange  (const int start , const int end , std::string& errMsg);

        bool getIntersectionOfRanges(std::map<int , RangeSet>& completeRangeSet , std::string& errMsg);
        std::string enumToName(ID_TYPE idType) const;
        bool allocateIDs(int& startingId , const int total_ids , std::string& errMsg);

        virtual ~RangeSet() { }

    protected :
        std::map < int , ID_TYPE > mRanges;/*int represent id , ID_TYPE represent whether its starting id of range or ending id of range or single id.*/
        const int mSlotNo;
};

#endif
