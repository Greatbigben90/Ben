#include "mood_face.h"

#include "avatar.h"
#include "debug.h"
#include "generic_factory.h"
#include "json.h"
#include "mutation.h"
#include "options.h"

namespace
{
generic_factory<mood_face> mood_face_factory( "mood_face" );
} // namespace

template<>
const mood_face &mood_face_id::obj() const
{
    return mood_face_factory.obj( *this );
}

/** @relates string_id */
template<>
bool mood_face_id::is_valid() const
{
    return mood_face_factory.is_valid( *this );
}

void mood_face::load_mood_faces( const JsonObject &jo, const std::string &src )
{
    mood_face_factory.load( jo, src );
}

void mood_face::reset()
{
    mood_face_factory.reset();
}

void mood_face::load( const JsonObject &jo, const std::string & )
{
    mandatory( jo, was_loaded, "values", values_ );
    std::sort( values_.begin(), values_.end(),
    []( const mood_face_value & valueA, const mood_face_value & valueB ) {
        return valueA.value() > valueB.value();
    } );
}

const std::vector<mood_face> &mood_face::get_all()
{
    return mood_face_factory.get_all();
}

void mood_face_value::load( const JsonObject &jo )
{
    mandatory( jo, was_loaded, "value", value_ );
    mandatory( jo, was_loaded, "face", face_ );
}

void mood_face_value::deserialize( JsonIn &jsin )
{
    JsonObject data = jsin.get_object();
    load( data );
}

const mood_face_id &avatar::character_mood_face()
{
    const bool option_horizontal = get_option<std::string>( "MORALE_STYLE" ) == "horizontal";
    if( mood_face_cache.has_value() && mood_face_horizontal == option_horizontal ) {
        return mood_face_cache.value();
    }

    mood_face_horizontal = option_horizontal;
    std::string face_type;
    for( const trait_id &mut : get_mutations() ) {
        if( !mut->threshold ) {
            continue;
        }
        face_type = mut.str();
        break;
    }

    // Valid: DEFAULT and DEFAULT_HORIZONTAL
    if( face_type.empty() ) {
        face_type = "DEFAULT";
    }

    if( mood_face_horizontal ) {
        face_type.append( "_HORIZONTAL" );
    }

    // Valid: THRESH_MUT and THRESH_MUT_HORIZONTAL
    // Example: THRESH_BIRD and THRESH_BIRD_HORIZONTAL
    for( const mood_face &face_id : mood_face::get_all() ) {
        if( face_id.getId().str() == face_type ) {
            mood_face_cache = face_id.getId();
            break;
        }
    }

    // If we didn't get it first try, we're not getting it again
    if( !mood_face_cache.has_value() ) {
        mood_face_cache = mood_face_id::NULL_ID();
        debugmsg( "No valid mood_face found for: %s", face_type );
    }

    return mood_face_cache.value();
}

void avatar::clear_mood_face()
{
    mood_face_cache.reset();
}