#pragma once

#include <steem/schema/schema.hpp>
#include <steem/protocol/schema_types.hpp>
#include <steem/chain/schema_types.hpp>

#include <steem/chain/database.hpp>

namespace steem { namespace chain {

using steem::schema::abstract_schema;

struct abstract_object;

struct index_info
   : public chainbase::index_extension
{
   index_info();
   virtual ~index_info();
   virtual std::shared_ptr< abstract_schema > get_schema() = 0;

   virtual void for_each_object_id( const database& db, std::function< int64_t >() cb ) = 0;
   virtual std::shared_ptr< abstract_object > object_from_binary( const std::vector<char>& binary_object ) = 0;
   virtual std::shared_ptr< abstract_object > object_from_json( const std::string& json_object ) = 0;
   virtual std::shared_ptr< abstract_object > object_from_db( const database& db, int64_t id ) = 0;
};

struct abstract_object
{
   abstract_object();
   virtual ~abstract_object();

   virtual void to_binary( std::vector<char>& binary_object ) = 0;
   virtual void to_json( std::string& json_object ) = 0;
   virtual void to_database( database& db ) = 0;
};

template< typename ValueType >
struct index_object_impl
   : public abstract_object
{
   virtual void to_binary( std::vector<char>& binary_object )
   {
      binary_object = fc::json::pack_to_vector( value );
   }

   virtual void to_json( std::string& json_object )
   {
      json_object = fc::json::to_string( value );
   }

   virtual void to_database( database& db )
   {
      db.create< ValueType >( [&]( ValueType& db_object )
      {
         db_object = value;
      } );
   }

   ValueType value;
};

template< typename MultiIndexType >
struct index_info_impl
   : public index_info
{
   typedef typename MultiIndexType::value_type value_type;

   index_info_impl()
      : _schema( steem::schema::get_schema_for_type< value_type >() ) {}
   virtual ~index_info_impl() {}

   virtual std::shared_ptr< abstract_schema > get_schema() override
   {   return _schema;   }

   virtual void for_each_object_id( const database& db, std::function< int64_t >() cb )
   {
      const auto& idx = db.get_index< MultiIndexType >().indices().get< by_id >();
      auto it = idx.begin();
      while( it != idx.end() )
      {
         cb( it->id._id );
         ++it;
      }
   }

   virtual void object_from_binary( const std::vector<char>& binary_object )
   {
      return std::static_pointer_cast< abstract_object >(
             std::make_shared< index_object_impl< value_type > >(
             fc::raw::unpack_from_vector( binary_object )
             ) );
   }

   virtual void object_from_json( const std::string& json_object )
   {
      fc::variant v = fc::json::from_string( json_object, fc::json::strict_parser );
      std::shared_ptr< index_object_impl< value_type > > result = std::make_shared< index_object_impl< value_type > >();
      fc::from_variant( v, result->value );
      return std::static_pointer_cast< abstract_object >( result );
   }

   virtual void object_from_db( const database& db, int64_t id )
   {
      return std::static_pointer_cast< abstract_object >(
             std::make_shared< index_object_impl< value_type > >(
             db.get< value_type, by_id >( value_type::id_type(id) )
             ) );
   }

   std::shared_ptr< abstract_schema > _schema;
};

template< typename MultiIndexType >
void _add_index_impl( database& db )
{
   db.add_index< MultiIndexType >();
   std::shared_ptr< chainbase::index_extension > ext =
      std::make_shared< index_info_impl< MultiIndexType > >();
   db.add_index_extension< MultiIndexType >( ext );
}

template< typename MultiIndexType >
void add_core_index( database& db )
{
   _add_index_impl< MultiIndexType >(db);
}

template< typename MultiIndexType >
void add_plugin_index( database& db )
{
   db._plugin_index_signal.connect( [&db](){ _add_index_impl< MultiIndexType >(db); } );
}

} }
