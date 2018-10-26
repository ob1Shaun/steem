
#include <steem/chain/database.hpp>
#include <steem/chain/load_state.hpp>

namespace steem { namespace chain {

// Version        : Must precisely match what is output by embedded code.
// Header         : JSON object that lists sections
// Section header : JSON object that lists count of objects
// Section footer : JSON object that lists hash of prior section
// Footer         : JSON object that lists sections and their hashes

// db_format_version : Must match STEEM_DB_FORMAT_VERSION
// network_type      : Must match STEEM_NETWORK_TYPE
// chain_id          : Must match requested chain ID and value of embedded GPO

struct steem_version_info
{
   std::string                          db_format_version;
   std::string                          network_type;
   std::map< std::string, std::string > object_schemas;
   std::map< std::string, fc::variant > config;
   chain_id_type                        chain_id;
};

std::string compute_embedded_version( const chain_id_type& chain_id )
{
   steem_version_info vinfo;
   vinfo.db_format_version = STEEM_DB_FORMAT_VERSION;
   vinfo.network_type = STEEM_NETWORK_TYPE;
   for_each_index_extension< index_info >( [&]( std::shared_ptr< index_extension > info )
   {
      std::shared_ptr< abstract_schema > sch = info->get_schema();
      std::string schema_name, str_schema;
      std::string str_schema;
      sch.get_name( schema_name );
      sch.get_str_schema( str_schema );
      vinfo.object_schemas.emplace_back( schema_name, str_schema );
   } );

   vinfo.chain_id = chain_id;
}

struct state_header
{
   // One section, objects
   steem_version_info            version;
   std::vector< std::string >    sections;
   std::
};

void init_genesis_from_state( const database& db, const std::string& state_filename )
{
   std::ifstream input_stream;
}

} }
