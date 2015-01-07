/***************************************************************************
 * Name    : id_Allocator.h                                                *
 *                                                                         *
 * Desc    : allocates ids , dynamically . Controls local id allocators.   *
 *                                                                         *
 * History : Dec 2014.                                                     *
 *                                                                         *
 * Initial version : Ambarish Kumar Shivam                                 *
 ***************************************************************************/

#ifndef __ID_ALLOCATOR__
#define __ID_ALLOCATOR__
#include"local_ID_Allocator.h"
#include<vector>

#define IDA_BLOCK (std::string("IDA::")+ __FUNCTION__ + "(): ")

class IDAllocator
{
    public:
        static IDAllocator* getInstance();
        virtual ~IDAllocator(){}
        bool getIDs(int & startingId , int total_ids , std::vector<int> slots , std::string& errMsg);
        bool saveID(int id , int slot , std::string& errMsg);
        Local_ID_Allocator& getLocal_ID_Allocator(const int slot);

        bool createIdAllocatorForNewSlot(const int slot , std::string& errMsg);

    private:
        IDAllocator() { }

        bool fillCompleteRangeSet(std::map<int , RangeSet>& completeRangeSet , const std::vector<int> slots , const int minRangeSize , std::string& errMsg) const;
        bool deleteAllocatedIDs(const int startingId , const std::vector<int> slots , const int totalIDs , std::string& errMsg);
        bool getCommonIDsFromSlots(int & startingId , int total_ids , std::vector<int>& slots , std::string& errMsg);
        void dumpIDstates(std::vector<int> slots , std::string & errMsg) const;
        bool getIDs_private(int & startingId , int total_ids , std::vector<int> slots , std::string& errMsg);
        bool saveID_private(int id , int slot , std::string& errMsg);
        bool areSlotsValid(std::vector<int>& slots , std::string& errMsg);
        bool isValidSlot(int slot , std::string& errMsg);

        std::map< int , Local_ID_Allocator > mLocal_ID_Allocator_map; /* int here , corresponds to the slot number.*/

        static IDAllocator* me;
};

#endif
