/***************************************************************************
 * Name    : fixedSizeRangeSet.h                                           *
 *                                                                         *
 * Desc    : api to store ids in form of ranges.                           *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#ifndef __FIXED_SIZE_RANGE_SET__
#define __FIXED_SIZE_RANGE_SET__

#include"rangeSet.h"

#define FS_RANGE_SET_BLOCK (std::string("FS_RangeSet::")+ __FUNCTION__ + "(): ")

class FixedSizeRangeSet : public RangeSet
{
    public :
        const int mRangeSize;

        FixedSizeRangeSet(const int start , const int end , const int sn) : RangeSet(sn) , mRangeSize(end - start + 1) {
            if(mRangeSize == 1)
            {
                HV_LOG( FS_RANGE_SET_BLOCK + " saving id " + toString(start) + " as single id " + " in slot " + toString(mSlotNo));
                mRanges[start] = SINGLE_ID;
            }
            else
            {
                HV_LOG( FS_RANGE_SET_BLOCK + " saving RANGE (" + toString(start) + "," + toString(end) + "). mRangeSize = " + toString(mRangeSize) + " in slot " + toString(mSlotNo));
                mRanges[start] = STARTING_ID;
                mRanges[end]   = ENDING_ID;
            }
        }
        virtual ~FixedSizeRangeSet() { }
        int  size() const { return mRanges.size(); }
        bool allocateIDs(int& rangeStart , std::string& errMsg) const;
        virtual bool isIDPresent(const int id) const;
        virtual bool deleteRange(const int start , const int end, std::string& errMsg);
        virtual bool saveRange(const int start , const int end, std::string& errMsg);

    private :
        bool findRangeStartAndEnd(const int start , const int end , int& rangeStart , int& rangeEnd , std::string& errMsg);
};

#endif
