/***************************************************************************
 * Name    : fixedSizeRangeSet.cpp                                         *
 *                                                                         *
 * Desc    : api to store ids in form of ranges.                           *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#include "id_Allocator.h"

bool FixedSizeRangeSet::allocateIDs(int& start , std::string& errMsg) const
{
    HV_LOG( FS_RANGE_SET_BLOCK + " mSlotNo = " + toString(mSlotNo));
    std::map < int , ID_TYPE >::const_iterator it = mRanges.begin();
    if(it == mRanges.end())
    {
        errMsg += "Range do not exists in map : " + toString(mRangeSize);
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }

    start = it->first;
    return true;
}

bool FixedSizeRangeSet::isIDPresent(const int id) const
{
    HV_LOG( FS_RANGE_SET_BLOCK + " id = " + toString(id) + " mSlotNo = " + toString(mSlotNo));
    if(mRangeSize == 1)
    {
        std::map < int , ID_TYPE >::const_iterator it = mRanges.find(id);
        if(it != mRanges.end())
        {
            return true;
        }
    }
    else
    {
        std::map < int , ID_TYPE >::const_iterator it = mRanges.begin();
        for( ; it != mRanges.end() ; it++)
        {
            int start = it->first;
            it++;
            if(it == mRanges.end())
            {
                HV_LOG(FS_RANGE_SET_BLOCK + " End id of start(" + toString(start) + ") is not present.");
                exit(1);
            }
            int end = it->first;
            if(start <= id && id <= end)
            {
                return true;
            }
        }
    }

    return false;
}

bool FixedSizeRangeSet::findRangeStartAndEnd(const int start , const int end , int& rangeStart , int& rangeEnd , std::string& errMsg)
{
    if(end < start)
    {
        errMsg += " end < start.";
        return false;
    }
    rangeStart = rangeEnd = 0;

    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(start);

    if(it != mRanges.end())
    {
        rangeStart = start;
        if(it->second == STARTING_ID)
        {
            it++;
            if(it == mRanges.end() || it->second != ENDING_ID)
            {
                errMsg += " End of range , for start (" + toString(start) + ") is missing.";
                return false;
            }
            rangeEnd = it->first;
        }
        else if(it->second == SINGLE_ID)
        {
            rangeStart = rangeEnd = start;
        }
        else
        {
            errMsg += " start is present as End_ID , hence given range do not exists.";
            return false;
        }
    }
    else/* search from begining.*/
    {
        it = mRanges.begin();
        while(it != mRanges.end())
        {
            rangeStart = it->first;
            if(it->second == SINGLE_ID)
            {
                rangeEnd = rangeStart;
            }
            else if(it->second == STARTING_ID)
            {
                it++;
                if(it == mRanges.end() || it->second != ENDING_ID)
                {
                    errMsg += " End of range , for start (" + toString(start) + ") is missing.";
                    return false;
                }
                rangeEnd = it->first;
            }
            else
            {
                errMsg += " Range Set is corrupted.";
                return false;
            }
            it++;
            if(rangeStart <= start && start <= rangeEnd)
            {
                break;
            }
        }
    }

    if(rangeStart <= start && rangeEnd >= end)
    {
        std::string f = "Range found = (" + toString(rangeStart) + "," + toString(rangeEnd) + "). Range given = (" + toString(start) + "," + toString(end) + ").";
        HV_LOG( FS_RANGE_SET_BLOCK + f);
        return true;
    }
    return false;
}
bool FixedSizeRangeSet::deleteRange(const int start , const int end, std::string& errMsg)
{
    HV_LOG( FS_RANGE_SET_BLOCK + " (" + toString(start) + "," + toString(end) + "). mRangeSize = " + toString(mRangeSize) + ", mSlotNo = " + toString(mSlotNo) );
    bool ok = true;
    if( end < start )
    {
        errMsg += " Attempt to delete invalid range.";
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }
    if(end - start + 1 > mRangeSize)
    {/* we accept subsets too.*/
        errMsg += " This range do not belong to me. my range size = " + toString(mRangeSize);
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }

    int rangeStart , rangeEnd;
    ok = findRangeStartAndEnd(start , end , rangeStart , rangeEnd , errMsg);
    if(!ok)
    {
        errMsg += "Could not find rangeStart/rangeEnd.";
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }

    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(rangeStart);

    if(mRangeSize == 1)
    {
        if(start != end)
        {
            errMsg += "start(" + toString(start) + ") is not equal to end(" + toString(end)+").";
            HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
            return false;
        }
        if(it->second != SINGLE_ID)
        {
            errMsg += "rangeStart(" + toString(rangeStart) + ") is not present as SINGLE_ID.";
            HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
            return false;
        }

        mRanges.erase(start);
        return true;
    }
    else
    {
        if(it->second != STARTING_ID)
        {
            errMsg += "start(" + toString(start) + ") is not present as STARTING_ID.";
            HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
            return false;
        }
    }

    it++;

    if(it == mRanges.end())
    {
        errMsg += "end of rangeStart(" + toString(rangeStart) + ") do not exists.";
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }
    else
    {
        if(it->second != ENDING_ID)
        {
            errMsg += "end(" + toString(it->second) + ") is not present as ENDING_ID.";
            HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
            return false;
        }   
    }

    HV_LOG(FS_RANGE_SET_BLOCK + "Erasing (" + toString(rangeStart) + "," + toString(it->first) + ").");
    mRanges.erase(rangeStart);
    mRanges.erase(it->first);

    {/*Save remaining if any.*/
        IDAllocator* ida = IDAllocator::getInstance();
        if(ida)
        {
            Local_ID_Allocator& lida = ida->getLocal_ID_Allocator(mSlotNo);
            if(lida.mSlotNo > 0){
                if(start > rangeStart)
                {
                    ok = lida.saveRange(rangeStart , start - 1 , errMsg);
                }
                if(end < rangeEnd)
                {
                    ok = lida.saveRange(end + 1 , rangeEnd , errMsg);
                }
            }
        }
    }

    return ok;
}

bool FixedSizeRangeSet::saveRange(const int start , const int end, std::string& errMsg)
{
    HV_LOG( FS_RANGE_SET_BLOCK + " (" + toString(start) + "," + toString(end) + "). mRangeSize = " + toString(mRangeSize) + " mSlotNo = " + toString(mSlotNo));
    if( end < start )
    {
        errMsg += " Attempt to save invalid range.";
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }
    if(end - start + 1 != mRangeSize)
    {
        errMsg += " This range do not belong to me. my range size = " + toString(mRangeSize);
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }
    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(start);
    if(it != mRanges.end())
    {
        errMsg += toString(start) + " already exists.";
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }
    it = mRanges.find(end);
    if(it != mRanges.end())
    {
        errMsg += toString(end) + " already exists.";
        HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
        return false;
    }

    if(mRangeSize == 1)
    {
        mRanges[start] = SINGLE_ID;
    }
    else
    {
        mRanges[start] = STARTING_ID;
        mRanges[end]   = ENDING_ID;
        it = mRanges.find(start);
        it++;
        if(it->first != end)/* Roll back;*/
        {
            mRanges.erase(start);
            mRanges.erase(end);
            errMsg += " one/more id(" + toString(it->first) +") in given range already exist.";
            HV_LOG( FS_RANGE_SET_BLOCK + errMsg);
            return false;
        }
    }

    return true;
}
