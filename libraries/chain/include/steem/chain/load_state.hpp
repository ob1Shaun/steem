#pragma once

namespace steem { namespace chain {

class database;

void init_genesis_from_state( const database& db, const std::string& state_filename );

} }
