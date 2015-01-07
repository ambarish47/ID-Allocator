/***************************************************************************
 * Name    : id_Allocator.cpp                                              *
 *                                                                         *
 * Desc    : Allocates ids , dynamically . Controls local id allocators.   *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#include"id_Allocator.h"

IDAllocator* IDAllocator::me = NULL;
Local_ID_Allocator null_object(0);

bool IDAllocator::getIDs ( int & startingId , int total_ids , std::vector<int> slots , std::string& errMsg )
{
    HV_LOG();

    removeDuplicateEntries(slots);

    if(areSlotsValid(slots , errMsg) == false)
    {
        errMsg += "One/more invalid slot given.";
        HV_LOG(IDA_BLOCK + errMsg);
        return false;
    }

    bool success = getIDs_private(startingId , total_ids , slots , errMsg);

    dumpIDstates(slots , errMsg);

    return success;
}

bool IDAllocator::saveID(int id , int slot , std::string& errMsg)
{
    HV_LOG();

    if(isValidSlot(slot , errMsg) == false)
    {
        errMsg += "Given slot is invalid.";
        HV_LOG(IDA_BLOCK + errMsg);
        return false;
    }

    if(isValidId(id , errMsg) == false)
    {
        errMsg += "Given ID is invalid.";
        HV_LOG(IDA_BLOCK + errMsg);
        return false;
    }

    bool success = saveID_private(id , slot , errMsg);

    std::vector<int> slots;
    slots.push_back(slot);
    dumpIDstates(slots , errMsg);

    return success;
}

bool IDAllocator::deleteAllocatedIDs(const int startingId , const std::vector<int> slots , const int totalIDs , std::string& errMsg)
{
    HV_LOG(IDA_BLOCK);
    std::map< int , Local_ID_Allocator >::iterator it ;
    std::vector<int>::const_iterator i = slots.begin();

    bool success = true;

    for( ; i != slots.end() && success ; i++)
    {
        it = mLocal_ID_Allocator_map.find(*i);
        if(it == mLocal_ID_Allocator_map.end())
        {
            errMsg += " IDs are NOT available in slot " + toString(*i);
            HV_LOG(IDA_BLOCK + errMsg);
            return false;
        }
        success = it->second.deleteCommonlyAllocatedIDs(startingId , totalIDs , errMsg);
    }

    return success;
}

bool IDAllocator::fillCompleteRangeSet(std::map<int , RangeSet>& completeRangeSet , const std::vector<int> slots , const int minRangeSize , std::string& errMsg) const
{
    HV_LOG(IDA_BLOCK);
    std::map< int , Local_ID_Allocator >::const_iterator it ;
    std::vector<int>::const_iterator i = slots.begin();

    bool success = true;

    for( ; i != slots.end() && success ; i++)
    {
        it = mLocal_ID_Allocator_map.find(*i);
        if(it == mLocal_ID_Allocator_map.end())
        {
            errMsg += " IDs are NOT available in slot " + toString(*i);
            HV_LOG(IDA_BLOCK + errMsg);
            return false;
        }
        success = it->second.fillCompleteRangeSet(completeRangeSet , minRangeSize , errMsg);
    }

    return success;
}

bool IDAllocator::getCommonIDsFromSlots(int & startingId , int total_ids , std::vector<int>& slots , std::string& errMsg)
{
    HV_LOG(IDA_BLOCK);
    bool success = true;

    std::map<int , RangeSet> completeRangeSet;/* int is slot.*/

    if(success){/* collect ids from slots.*/
        success = fillCompleteRangeSet(completeRangeSet , slots , total_ids , errMsg);
        if(!success)
        {
            errMsg += " fillCompleteRangeSet failed.";
            HV_LOG(IDA_BLOCK + errMsg);
            return false;
        }
        std::map<int , RangeSet>::const_iterator it = completeRangeSet.begin();
        HV_LOG("\n" + IDA_BLOCK + " following are dumps of COPIED ranges having size > total_ids , across slots.");
        for( ; it != completeRangeSet.end() ; it++)
        {
            it->second.dumpIDstates(errMsg);
        }
    }

    RangeSet intersection(0);
    if(success){/* get intersection.*/
        success = intersection.getIntersectionOfRanges(completeRangeSet , errMsg);
        if(!success)
        {
            errMsg += " getIntersectionOfRanges failed.";
            HV_LOG(IDA_BLOCK + errMsg);
            return false;
        }
        HV_LOG(IDA_BLOCK + " following is dump of INTERSECTION of COPIED ranges , across slots.");
        intersection.dumpIDstates(errMsg);
    }

    if(success){/* allocate ids.*/
        success = intersection.allocateIDs(startingId , total_ids , errMsg);
        if(!success)
        {
            errMsg += " intersection.allocateIDs failed.";
            HV_LOG(IDA_BLOCK + errMsg);
            return false;
        }
    }

    if(success) /* delete allocated ids from slots.*/
    {
        success = deleteAllocatedIDs(startingId , slots , total_ids , errMsg);
    }

    return success;
}

bool IDAllocator::getIDs_private ( int & startingId , int total_ids , std::vector<int> slots , std::string& errMsg )
{
    std::string s_slots = " ";
    for(std::vector<int>::const_iterator i = slots.begin() ; i != slots.end() ; i++)
    {
        s_slots += toString(*i) + " ";
    }
    HV_LOG(IDA_BLOCK + " total_ids = " + toString(total_ids) + ". slots =" + s_slots);
    bool success = true;

    if(slots.size() > 1)
    {
        bool success = getCommonIDsFromSlots(startingId , total_ids , slots , errMsg);
    }
    else
    {
        std::map< int , Local_ID_Allocator >::iterator it = mLocal_ID_Allocator_map.find(slots[0]);
        if( it != mLocal_ID_Allocator_map.end())
        {
            success = it->second.getLocalIDs(startingId , total_ids , errMsg);
        }
        if(!success)
        {
            std::cout << total_ids << " consecutive Ids are not available in slot : " << slots[0] << std::endl;
            std::cout << " errMsg = " << errMsg << std::endl;
            HV_LOG(IDA_BLOCK + errMsg);
            return false;
        }
    }

    return success;
}


bool IDAllocator::saveID_private(int id , int slot , std::string& errMsg)
{
    HV_LOG(IDA_BLOCK + " id = " + toString(id) + ". slot = " + toString(slot));
    std::map< int , Local_ID_Allocator >::iterator it = mLocal_ID_Allocator_map.find(slot);
    if(it == mLocal_ID_Allocator_map.end())
    {
        errMsg += " IDs are NOT available given slot " + toString(slot);
        HV_LOG(IDA_BLOCK + errMsg);
        return false;
    }
    bool success = it->second.saveLocalID(id , errMsg);
    if(!success)
    {
        HV_LOG(IDA_BLOCK + errMsg);
        return false;
    }
    return true;
}
int main()
{
    IDAllocator *anIDAllocator = IDAllocator::getInstance();
    int task , id;
    char task_c[100];

    std::cout << "          1 = getID()" << std::endl;
    std::cout << "          2 = saveID()" << std::endl;
    std::cout << "          3 = create ID Allocator." << std::endl;
    while(true)
    {
        std::string errMsg;
        std::cout << std::endl << " task = ";
        std::cin >> task_c;
        task = atoi(task_c);

        switch(task)
        {
            case 1:
                {
                    std::cout << " enter total consecutive Ids : ";
                    int totalConsIds = 0 ;
                    std::cin >> totalConsIds;
                    int slot = 1; char ch = 0;
                    std::vector<int> slots;
                    std::cout << " enter slot number(s) (separated by space)" ;
                    scanf("%d%c" , &slot , &ch);

                    slots.push_back(slot);
                    while( ch != '\n')
                    {
                        scanf("%d%c" , &slot , &ch);
                        slots.push_back(slot);
                    }
                    int startingId = -1;
                    if(anIDAllocator->getIDs(startingId , totalConsIds , slots , errMsg))
                    {
                        std::cout << std::endl << " Alloted ids = " << startingId << " to " << startingId + totalConsIds - 1 << "." << std::endl;
                    }
                    else
                    {
                        std::cout << std::endl << errMsg << std::endl;
                        std::cout << " If invalid slot error came, then create ID Allocator first for that slot by entering 3." << std::endl;
                    }
                }
                break;
            case 2:
                {
                    std::cout << " Give ID : ";
                    std::cin >> id;
                    int slot = 1;
                    std::cout << " Give slot number : ";
                    std::cin >> slot;
                    std::cout << std::endl << " Saving Id " << id << std::endl;

                    if(anIDAllocator->saveID(id , slot , errMsg) == false)
                    {
                        std::cout<< " Saving ID failed . error = " << errMsg << std::endl;
                        std::cout << " If invalid slot err came , then create ID Allocator first for that slot by pressing 3." << std::endl;
                    }
                }
                break;
            case 3 :
                {
                    std::cout << " Give slot number : ";
                    int slot;
                    std::cin >> slot;
                    if(anIDAllocator->createIdAllocatorForNewSlot(slot , errMsg))
                    {
                        std::cout << std::endl << " successfully created allocator. Valid Available range is [1,16000]." << std::endl;
                    }
                    else
                    {
                        std::cout<< " failed to create allocator. error = " << errMsg << std::endl;
                    }
                }
                break;
            default:
                std::cout<< " Enter 1 or 2 or 3 . You gave "<< task << std::endl;
                break;
        }
    }
}
IDAllocator* IDAllocator::getInstance()
{
    if(!me)
    {
        me = new IDAllocator();
    }

    return me;
}
Local_ID_Allocator& IDAllocator::getLocal_ID_Allocator(const int slot)
{
    if(slot <= 0) return null_object;
    std::map< int , Local_ID_Allocator >::iterator it = mLocal_ID_Allocator_map.find(slot);

    if(it != mLocal_ID_Allocator_map.end())
    {
        return it->second;
    }
    return null_object;
}

void IDAllocator::dumpIDstates(std::vector<int> slots , std::string & errMsg) const
{
    if(slots.size() <= 0) return;

    std::map< int , Local_ID_Allocator >::const_iterator it;
    std::vector<int>::const_iterator slot = slots.begin();

    for( ; slot != slots.end() ; slot++)
    {
        it = mLocal_ID_Allocator_map.find((*slot));
        if(it == mLocal_ID_Allocator_map.end())
        {
            errMsg += "IDAllocator::dumpIDstates() slot(" + toString((*slot)) + ") is not valid.";
            continue;
        }
        it->second.dumpIDstates(errMsg);
    }
}

bool IDAllocator::isValidSlot(int slot , std::string& errMsg)
{
    std::map< int , Local_ID_Allocator >::const_iterator it = mLocal_ID_Allocator_map.find(slot);
    if(it == mLocal_ID_Allocator_map.end())
    {
        errMsg += " No ID Allocator found for slot " + toString(slot) + ".";
        return false;
    }
    return true;
}

bool IDAllocator::areSlotsValid(std::vector<int>& slots , std::string& errMsg)
{
    for(int i = 0 ; i < slots.size() ; i++)
    {
        if(!isValidSlot(slots[i] , errMsg))
        {
            return false;
        }
    }
    return true;
}

bool IDAllocator::createIdAllocatorForNewSlot(const int slot , std::string& errMsg)
{
    mLocal_ID_Allocator_map.insert(std::pair<int , Local_ID_Allocator>(slot , Local_ID_Allocator(slot)));

    return true;
}
