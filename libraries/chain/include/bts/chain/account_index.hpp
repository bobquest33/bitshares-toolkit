#pragma once
#include <bts/chain/index.hpp>
#include <bts/chain/account_object.hpp>

namespace bts { namespace chain {
    
   /**
    *  @class account_index 
    *  @brief enable account lookup by NAME or INSTANCE and enforce uniqueness
    */
   class account_index : public index
   {
      public:
         typedef account_object object_type;

         ~account_index(){}

         virtual int64_t size()const;

         virtual void modify( const object* obj, const std::function<void(object*)>& m )override;
         virtual void add( unique_ptr<object> o )override;
         virtual void remove_after( object_id_type id )override;
         virtual void remove( object_id_type id )override;
         virtual const object* get( object_id_type id )const override;

         const account_object* get( const string& name )const;

      private:
         vector<unique_ptr<account_object> >   accounts;
         unordered_map<string,account_object*>  name_to_id;
   };

} } 