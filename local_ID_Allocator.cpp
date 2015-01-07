/***************************************************************************
 * Name    : local_ID_Allocator.cpp                                        *
 *                                                                         *
 * Desc    : allocates local ids , dynamically.                            *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#include "local_ID_Allocator.h"

bool Local_ID_Allocator::fillCompleteRangeSet(std::map<int , RangeSet>& completeRangeSet , int minRangeSize , std::string& errMsg) const
{
    HV_LOG(LIDA_BLOCK);
    bool success = true;

    minRangeSize = getFirstNonEmptySet(minRangeSize);/*minRangeSize may not be available , but greater than minRangeSize amy be available.*/
    std::map < int , FixedSizeRangeSet >::const_iterator it = mMap_ConsecutiveIDs.find(minRangeSize);

    if(it == mMap_ConsecutiveIDs.end())
    {
        errMsg += " Given number of consecutive IDs are not available. minRangeSize = " + toString(minRangeSize);
        HV_LOG( LIDA_BLOCK + errMsg );
        return false;
    }

    RangeSet newRangeSet(mSlotNo);/* This rangeSet belongs to my slot.*/

    for( ; it != mMap_ConsecutiveIDs.end() && success ; it++)
    {
        success = it->second.fillCompleteRangeSet(newRangeSet , errMsg);
    }

    if(success)
    {
        completeRangeSet.insert(std::pair<int , RangeSet> ( mSlotNo , newRangeSet ) );
    }
    return success;
}

bool Local_ID_Allocator::get_RangeToBeSaved_And_delete_PredecessorRangeAndSuccessorRange(int& rangeStart , int& rangeEnd , const int id , std::string& errMsg )
{
    HV_LOG( LIDA_BLOCK + " Range = (" + toString(rangeStart) + "," + toString(rangeEnd) + ") id = " + toString(id));
    rangeStart = id , rangeEnd = id;
    bool ok = true , predecessorFound = false , successorFound = false;

    std::map < int , FixedSizeRangeSet >::iterator mapIter = mMap_ConsecutiveIDs.begin();

    for( ; mapIter != mMap_ConsecutiveIDs.end() ; mapIter++)
    {/* There can be max sqrt(n) members in mMap_ConsecutiveIDs. very unlikely.*/
        FixedSizeRangeSet& FixedSizeRangeSet = mapIter->second; 

        if(predecessorFound && successorFound)
        {
            break;
        }
        if(predecessorFound == false)
        {/* find predecessor and hence find rangeStart.*/
            if(FixedSizeRangeSet.isPredecessorPresent(id))
            {
                HV_LOG( LIDA_BLOCK + " predecessor is found.");
                predecessorFound = true;
                ok = FixedSizeRangeSet.getPredecessorRangeStart(id , rangeStart , errMsg);
                ok = FixedSizeRangeSet.deleteRange(rangeStart , id - 1 , errMsg);
                if(FixedSizeRangeSet.size() <= 0)
                {
                    mMap_ConsecutiveIDs.erase(mapIter);
                }
            }
        }
        if(successorFound == false)
        {
            if(FixedSizeRangeSet.isSuccessorPresent(id))
            {
                HV_LOG( LIDA_BLOCK + " successor is found.");
                successorFound = true;
                ok = FixedSizeRangeSet.getSuccessorRangeEnd(id , rangeEnd , errMsg);
                ok = FixedSizeRangeSet.deleteRange(id + 1 , rangeEnd , errMsg);
                if(FixedSizeRangeSet.size() <= 0)
                {
                    mMap_ConsecutiveIDs.erase(mapIter);
                }
            }
        }
    }

    return ok;
}

bool Local_ID_Allocator::saveRange(const int rangeStart , const int rangeEnd, std::string& errMsg)
{
    int rangeSize = rangeEnd - rangeStart + 1;
    if(rangeSize < 1)
    {
        errMsg += " rangeSize < 1.";
        HV_LOG( LIDA_BLOCK + errMsg );
        return false;
    }
    bool ok = true;

    HV_LOG( LIDA_BLOCK + "Saving range (" + toString(rangeStart) + "," + toString(rangeEnd) + ") in map = " + toString(rangeSize) + ", slot = " + toString(mSlotNo));

    std::map < int , FixedSizeRangeSet >::iterator it = mMap_ConsecutiveIDs.find(rangeSize);
    if(it != mMap_ConsecutiveIDs.end())
    {
        FixedSizeRangeSet& FixedSizeRangeSet = it->second;
        ok = FixedSizeRangeSet.saveRange(rangeStart , rangeEnd , errMsg);
    }
    else
    {
        mMap_ConsecutiveIDs.insert(std::pair<int , FixedSizeRangeSet> (rangeSize , FixedSizeRangeSet(rangeStart , rangeEnd , mSlotNo)) );
    }

    return ok;
}

bool Local_ID_Allocator::saveLocalID(const int id , std::string& errMsg )
{
    HV_LOG();
    HV_LOG( LIDA_BLOCK + " id = " + toString(id));
    if(isIDAlreadyPresent(id))
    {
        errMsg += " Attempt to save an id(" + toString(id) + ") , which already exists.";
        HV_LOG( LIDA_BLOCK + errMsg );
        return false;
    }

    int rangeStart = id , rangeEnd = id;
    bool success = get_RangeToBeSaved_And_delete_PredecessorRangeAndSuccessorRange(rangeStart , rangeEnd ,id , errMsg);
    if(!success)
    {
        HV_LOG( LIDA_BLOCK + errMsg );
        return false;
    }

    success = saveRange(rangeStart , rangeEnd , errMsg);/*THIS MUST SUCCEED. else IDs will be lost.*/

    return success;
}

bool Local_ID_Allocator::allocateIDs(int & rangeStart , const int rangeSize , std::string& errMsg)
{
    HV_LOG(LIDA_BLOCK + "rangeSize = " + toString(rangeSize) + ", rangeStart = " + toString(rangeStart));
    std::map < int , FixedSizeRangeSet >::iterator mapIter = mMap_ConsecutiveIDs.find(rangeSize);
    FixedSizeRangeSet& FixedSizeRangeSet = mapIter->second;/*mapIter is already tested.*/

    bool ok = FixedSizeRangeSet.allocateIDs(rangeStart , errMsg);
    if(ok == false)
    {
        HV_LOG( LIDA_BLOCK + errMsg);
        exit(1);
    }

    return true;
}

bool Local_ID_Allocator::deleteAllocatedIds(const int rangeSize , const int start , const int end , std::string& errMsg)
{
    HV_LOG(LIDA_BLOCK + "rangeSize = " + toString(rangeSize) + ", start = " + toString(start) + ", end = " + toString(end));
    std::map < int , FixedSizeRangeSet >::iterator mapIter = mMap_ConsecutiveIDs.find(rangeSize);
    if(mapIter == mMap_ConsecutiveIDs.end())
    {
        errMsg += "rangeSize(" + toString(rangeSize) + ") do not exist.";
        HV_LOG(LIDA_BLOCK + errMsg);
        return false;
    }
    FixedSizeRangeSet& FixedSizeRangeSet = mapIter->second;/*mapIter is already tested.*/

    bool ok = FixedSizeRangeSet.deleteRange(start , end , errMsg);
    if(ok && FixedSizeRangeSet.size() <= 0)
    {/* delete corresponding map member. */
        mMap_ConsecutiveIDs.erase(mapIter);
    }

    return ok;
}

bool Local_ID_Allocator::deleteCommonlyAllocatedIDs(const int start , const int totalIDs , std::string& errMsg)
{
    /* ALGORITHM :
     *  Get map from mMap_ConsecutiveIDs which contains start.
     *  Delete Allocated IDs.
     *  Save remaining IDs , if any.
     */
    HV_LOG(LIDA_BLOCK + "start = " + toString(start) + ", totalIDs = " + toString(totalIDs));
    int rangeSize = 0;
    std::map < int , FixedSizeRangeSet >::const_iterator mapIter = mMap_ConsecutiveIDs.begin();
    for(; mapIter != mMap_ConsecutiveIDs.end() ; mapIter++)
    {
        if(mapIter->second.isIDPresent(start))
        {
            rangeSize = mapIter->first;
            break;
        }
    }
    HV_LOG(LIDA_BLOCK + " rangeSize = " + toString(rangeSize));
    if(rangeSize <= 0)
    {
        errMsg += "start("+ toString(start) + ") do not exists in slot " + toString(mSlotNo);
        HV_LOG( LIDA_BLOCK + errMsg );
        return false;
    }

    /* Delete allocated IDs. Below function takes care of saving remaining IDs.*/
    if(deleteAllocatedIds(rangeSize , start , start + totalIDs - 1 , errMsg) == false)
    {
        errMsg += " deletion of allocated ids failed.";
        HV_LOG(LIDA_BLOCK + errMsg);
        return false;
    }

    return true;
}

bool Local_ID_Allocator::getLocalIDs(int& start , const int total_ids , std::string& errMsg)
{ /* returns consecutive IDs locally from mSlot.*/
    /*
     *    ALGORITHM :
     *    Get First Non-empty map from mMap_ConsecutiveIDs, for total_ids.
     *    Allocate IDs.
     *    Delete Allocated IDs.
     */
    HV_LOG( LIDA_BLOCK + " requested total_ids = " + toString(total_ids));

    int rangeSize = getFirstNonEmptySet(total_ids);
    HV_LOG(LIDA_BLOCK + " rangeSize = " + toString(rangeSize));
    if( rangeSize <= 0)
    {
        errMsg += " No given cont. id set are available in given slot for request no. of IDs = " + toString(total_ids);
        HV_LOG( LIDA_BLOCK + errMsg );
        return false;
    }

    if(allocateIDs(start , rangeSize , errMsg) == false)
    {
        errMsg += "Allocation failed. rangeSize = " + toString(rangeSize);
        HV_LOG(LIDA_BLOCK + errMsg);
        return false;
    }

    /* Delete allocated IDs. Below function takes care of saving remaining IDs.*/
    if(deleteAllocatedIds(rangeSize , start , start+total_ids-1 , errMsg) == false)
    {
        errMsg += " deletion of allocated ids failed.";
        HV_LOG(LIDA_BLOCK + errMsg);
        return false;
    }

    return true;
}

int Local_ID_Allocator::getFirstNonEmptySet(const int total_ids) const
{/* start searching from begining. */
    HV_LOG(LIDA_BLOCK + " total_ids = " + toString(total_ids) + ", slot = " + toString(mSlotNo));
    if(total_ids <= 0)
    {
        HV_LOG(LIDA_BLOCK + " total_ids <= 0.");
        return 0;
    }
    std::map < int , FixedSizeRangeSet >::const_iterator mapIter = mMap_ConsecutiveIDs.begin();
    while(mapIter != mMap_ConsecutiveIDs.end())
    {
        if(mapIter->first >= total_ids)
        {
            HV_LOG(LIDA_BLOCK + " returning set number : " + toString(mapIter->first));
            return mapIter->first;
        }
        mapIter++;
    }

    HV_LOG(LIDA_BLOCK + " returning 0 at end.");
    return 0;
}

bool Local_ID_Allocator::isIDAlreadyPresent(const int id) const
{
    std::map < int , FixedSizeRangeSet >::const_iterator iter = mMap_ConsecutiveIDs.begin();

    for( ; iter != mMap_ConsecutiveIDs.end() ; iter++)
    {
        if(iter->second.isIDPresent(id))
        {
            return true;
        }
    }

    return false;
}

void Local_ID_Allocator::dumpIDstates(std::string & errMsg) const
{
    std::map < int , FixedSizeRangeSet >::const_iterator iter = mMap_ConsecutiveIDs.begin();
    for( ; iter != mMap_ConsecutiveIDs.end() ; iter++)
    {
        iter->second.dumpIDstates(errMsg , iter->first);
    }
}
