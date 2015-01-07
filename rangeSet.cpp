/***************************************************************************
 * Name    : range_set.cpp                                                 *
 *                                                                         *
 * Desc    : api to store ids in form of ranges.                           *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#include "rangeSet.h"
#include<fstream>

bool RangeSet::allocateIDs(int& startingId , const int total_ids , std::string& errMsg)
{
    /* ALGORITHM :
     *  checks for possible errors.
     *  sets startingId to first id in range Set WHICH HAS MINIMUM SIZE.
     * */
    HV_LOG(RANGE_SET_BLOCK + "total_ids = " + toString(total_ids) + ", mSlotNo = " + toString(mSlotNo));

    int minRangeSize = MAX_ID + 1 , tempStart = 0;

    std::map<int , ID_TYPE>::iterator it = mRanges.begin();

    /*Select minimum size range.*/
    for( ; it != mRanges.end() ; it++)
    {
        if(it->second == SINGLE_ID)
        {
            if(total_ids == 1)
            {
                startingId = it->first;
                return true;
            }
            continue;
        }
        else if(it->second == STARTING_ID)
        {
            int start = it->first;

            it++;
            if(it == mRanges.end())
            {
                errMsg += "Range end for start(" + toString(start) + ") do not exists.";
                HV_LOG( RANGE_SET_BLOCK + errMsg);
                return false;
            }
            int end = it->first;
            if(it->second != ENDING_ID)
            {
                errMsg += "Range end (" + toString(end) + ") is not of type ENDING_ID.";
                HV_LOG( RANGE_SET_BLOCK + errMsg);
                return false;
            }

            int size = end - start + 1;
            if(size >= total_ids )
            {
                if(minRangeSize > size)
                {
                    minRangeSize = size;
                    tempStart = start;
                }
            }
        }
    }
    if(tempStart > 0)
    {
        startingId = tempStart;
        return true;
    }

    HV_LOG(RANGE_SET_BLOCK + " returning false. tempStart = " + toString(tempStart) + ", minRangeSize = " + toString(minRangeSize));
    return false;
}

bool RangeSet::getIntersectionOfRanges(std::map<int , RangeSet>& completeRangeSet , std::string& errMsg)
{
    HV_LOG( RANGE_SET_BLOCK + "begins. mSlotNo = " + toString(mSlotNo));
    while(true)
    {
        std::map<int , RangeSet>::iterator slotIter = completeRangeSet.begin();
        if(slotIter == completeRangeSet.end())
        {
            errMsg += "completeRangeSet is empty.";
            HV_LOG( RANGE_SET_BLOCK + errMsg);
            return true;
        }

        int maxStart = 0 , minEnd = MAX_ID + 1 , minEndSlot ;

        for( ; slotIter != completeRangeSet.end() ; slotIter++)
        {
            RangeSet& rs = slotIter->second;

            std::map<int , ID_TYPE>::const_iterator rangeIter = rs.mRanges.begin();

            if(rangeIter == rs.mRanges.end())
            {
                HV_LOG( RANGE_SET_BLOCK + toString(slotIter->first) + " slot's range becomes empty. Returning true.");
                return true;
            }

            int rangeStart = 0 , rangeEnd = 0;
            if(rangeIter->second == SINGLE_ID)
            {
                rangeStart = rangeEnd = rangeIter->first;
            }
            else if(rangeIter->second == STARTING_ID)
            {
                rangeStart = rangeIter->first;

                rangeIter++;

                rangeEnd = rangeIter->first;
            }
            else
            {
                errMsg += " Begining of rangeSet should not be ENDING Id. Asserting. id = ";
                errMsg += toString(rangeIter->first) + ", slot = " + toString(slotIter->first);
                HV_LOG(RANGE_SET_BLOCK + errMsg);
                exit(1);
            }

            if(maxStart < rangeStart)
            {
                maxStart = rangeStart;
            }
            if(minEnd > rangeEnd)
            {
                minEnd = rangeEnd;
                minEndSlot = slotIter->first;
            }
        }/* end for loop*/
        std::string intersectionInfo = "maxStart = " + toString(maxStart) + ", minEnd = " + toString(minEnd) + ", minEndSlot = " + toString(minEndSlot) ;
        HV_LOG("      " + intersectionInfo);

        {/* Insert common range in intersection.*/
            if(maxStart < minEnd)
            {
                HV_LOG("      Adding range (" + toString(maxStart) + "," + toString(minEnd) + ") in intersection.");
                mRanges.insert(std::pair<int , ID_TYPE> ( maxStart , STARTING_ID));
                mRanges.insert(std::pair<int , ID_TYPE> ( minEnd , ENDING_ID));
            }
            else if(maxStart == minEnd)
            {
                HV_LOG("      Adding single id " + toString(minEnd) + " in intersection.");
                mRanges.insert(std::pair<int , ID_TYPE> ( minEnd , SINGLE_ID));
            }
        }

        {/* Delete first range of minEndSlot.*/
            std::map<int , RangeSet>::iterator slotIter = completeRangeSet.find(minEndSlot);
            if(slotIter == completeRangeSet.end())
            {
                errMsg += "Asserting. No slot contains minimum end id. minEndSlot = " + toString(minEndSlot);
                HV_LOG(RANGE_SET_BLOCK + errMsg);
                exit(1);
            }
            RangeSet& rs = slotIter->second;
            std::map<int , ID_TYPE>& ranges = rs.mRanges;
            std::map<int , ID_TYPE>::iterator rangeIter = ranges.begin();

            if(rangeIter->second == SINGLE_ID)
            {
                HV_LOG("      erasing single id " + toString(rangeIter->first) + " from "  + toString(slotIter->first));
                ranges.erase(rangeIter->first);
            }
            else if(rangeIter->second == STARTING_ID)
            {
                int start = rangeIter->first;
                rangeIter++;
                int end = rangeIter->first;
                HV_LOG("      erasing range (" + toString(start) +","+ toString(end) + ") from " + toString(slotIter->first) );
                ranges.erase(start);
                ranges.erase(end);
            }
            else
            {
                HV_LOG(RANGE_SET_BLOCK + "Asserting. range set starts with ending id.");
                exit(1);
            }
            if(ranges.size() <= 0)/* erase range set.*/
            {
                HV_LOG("      Range set " + toString(slotIter->first) + " becomes empty. stopping intersection.");
                break;/* Since one range is empty , there can not be more intersections.*/
                //completeRangeSet.erase(slotIter);
            }
        }
    }

    HV_LOG( RANGE_SET_BLOCK + "ends. mSlotNo = " + toString(mSlotNo));
    return true;
}

bool RangeSet::fillCompleteRangeSet(RangeSet& completeRangeSet , std::string& errMsg) const
{
    HV_LOG( RANGE_SET_BLOCK + " mSlotNo = " + toString(mSlotNo));
    bool success = true;

    std::map < int , ID_TYPE >::const_iterator it = mRanges.begin();
    for( ; it != mRanges.end() ; it++)
    {
        completeRangeSet.mRanges.insert(std::pair< int , ID_TYPE> (it->first , it->second));
    }
    return success;
}

bool RangeSet::getPredecessorRangeStart(const int id , int& rangeStart , std::string& errMsg) const
{
    HV_LOG(RANGE_SET_BLOCK + "id = " + toString(id) + ", rangeStart = " + toString(rangeStart) + ", mSlotNo = " + toString(mSlotNo));
    int predecessor = id - 1;
    if(isValidId(predecessor , errMsg) == false)
    {
        return false;
    }

    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(predecessor);
    if(it == mRanges.end())
    {
        errMsg += " Predecessor( " + toString(predecessor) + " not found.";
        HV_LOG( RANGE_SET_BLOCK + errMsg);
        return false;
    }

    if(it->second == SINGLE_ID)
    {
        rangeStart = predecessor;
        return true;
    }
    else if(it->second == ENDING_ID)
    {
        it--;

        if(it->second != STARTING_ID)
        {
            errMsg += " Predecessor range start(" + toString(it->first) +") is not present as STARTING_ID. Asserting.";
            HV_LOG( RANGE_SET_BLOCK + errMsg);
            exit(1);
        }
        rangeStart = it->first;
        return true;
    }
    else
    {
        errMsg += " Predecessor(" + toString(it->first) +") is present as STARTING_ID. This Means Given ID already available.";
        HV_LOG( RANGE_SET_BLOCK + errMsg);
        return false;
    }

    return false;
}

bool RangeSet::getSuccessorRangeEnd(const int id , int& rangeEnd , std::string& errMsg) const
{
    HV_LOG(RANGE_SET_BLOCK + "id = " + toString(id) + ", rangeEnd = " + toString(rangeEnd) + ", mSlotNo = " + toString(mSlotNo));
    int successor = id + 1;
    if(isValidId(successor , errMsg) == false)
    {
        return false;
    }

    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(successor);
    if(it == mRanges.end())
    {
        errMsg += " Successor( " + toString(successor) + " not found.";
        HV_LOG( RANGE_SET_BLOCK + errMsg);
        return false;
    }

    if(it->second == SINGLE_ID)
    {
        rangeEnd = successor;
        return true;
    }
    else if(it->second == STARTING_ID)
    {
        it++;

        if(it == mRanges.end())
        {
            errMsg += " successor range end is not present. Asserting.";
            HV_LOG( RANGE_SET_BLOCK + errMsg);
            exit(1);
        }
        if(it->second != ENDING_ID)
        {
            errMsg += " successor range end(" + toString(it->first) +") is not present as ENDING_ID. Asserting.";
            HV_LOG( RANGE_SET_BLOCK + errMsg);
            exit(1);
        }
        rangeEnd = it->first;
        return true;
    }
    else
    {
        errMsg += " successor(" + toString(it->first) +") is present as ENDING_ID. This means Given ID already available.";
        HV_LOG( RANGE_SET_BLOCK + errMsg);
        return false;
    }
    return false;
}

bool RangeSet::isPredecessorPresent(const int id) const
{
    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(id - 1);
    if(it == mRanges.end())
    {
        return false;
    }

    return true;
}
bool RangeSet::isSuccessorPresent(const int id) const
{
    std::map < int , ID_TYPE >::const_iterator it = mRanges.find(id + 1);
    if(it == mRanges.end())
    {
        return false;
    }   

    return true;
}

bool RangeSet::isIDPresent(const int id) const
{
    HV_LOG( RANGE_SET_BLOCK + " NOT implemented." );
    bool ok = true;
    return ok;
}

bool RangeSet::deleteRange(const int start , const int end, std::string& errMsg)
{
    HV_LOG( RANGE_SET_BLOCK + " NOT implemented." );
    bool ok = true;
    return ok;
}

bool RangeSet::saveRange(const int start , const int end, std::string& errMsg)
{
    HV_LOG( RANGE_SET_BLOCK + " NOT implemented." );
    bool ok = true;
    return ok;
}

bool RangeSet::dumpIDstates(std::string & errMsg , const int rangeSize) const
{
    std::ofstream out;
    out.open("/tmp/dynamicIDstatesLog" , std::ios::binary | std::ios::out | std::ios::app);

    if(!out)
    {
        errMsg += " could not open output file : /tmp/dynamicIDstates.";
        std::cout << errMsg << std::endl;
        HV_LOG( RANGE_SET_BLOCK + errMsg);
        return false;
    }

    out << "============= slot = " << mSlotNo << " rangeSize = " << rangeSize << " =============" << std::endl;

    std::map < int , ID_TYPE >::const_iterator iter = mRanges.begin();

    for( ; iter != mRanges.end() ; iter++ )
    {
        if(iter->second == SINGLE_ID)
        {
            out << "(" << iter->first << "," << iter->first << ") ";
        }
        else if(iter->second == STARTING_ID)
        {
            out << "(" << iter->first << ",";

            iter++;

            if(iter == mRanges.end() )
            {
                out << "?.) ";
                return false;
            }
            else if (iter->second != ENDING_ID)
            {
                out << "?) "<< std::endl;
                continue;
            }
            else
            {
                out << iter->first << ") " ;
            }
        }
        else
        {
            out << "(?," << iter->first << ") " << std::endl;
        }
    }
    out << std::endl << "==================================================" << std::endl;
    out.close();

    return true;
}

std::string RangeSet::enumToName(ID_TYPE idType) const
{
    switch(idType)
    {
        case SINGLE_ID :
            return "SINGLE_ID";
            break;
        case STARTING_ID :
            return "STARTING_ID";
            break;
        case ENDING_ID :
            return "ENDING_ID";
            break;
        default :
            return "UNKNOWN_TYPE";
    }
}
