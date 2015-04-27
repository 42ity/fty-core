#include "utils_app.h"

// TODO: move string_to_X functioncs into utils.[hc] as soon as streq conflict is solved

int64_t string_to_int64( const char *value )
{
    char *end;
    int64_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return INT64_MAX; }
    result = strtoll( value, &end, 10 );
    if( *end ) errno = EINVAL;
    if( errno ) return INT64_MAX;
    return result;
}

int32_t string_to_int32( const char *value )
{
    char *end;
    int32_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return INT32_MAX; }
    result = strtol( value, &end, 10 );
    if( *end ) errno = EINVAL;
    if( errno ) return INT32_MAX;
    return result;
}
    
uint64_t string_to_uint64( const char *value )
{
    char *end;
    uint64_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return UINT64_MAX; }
    result = strtoull( value, &end, 10 );
    if( *end ) errno = EINVAL;
    if( errno ) return UINT64_MAX;
    return result;
}

uint32_t string_to_uint32( const char *value )
{
    char *end;
    uint32_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return UINT32_MAX; }
    result = strtoul( value, &end, 10);
    if( *end ) errno = EINVAL;
    if( errno ) { return UINT32_MAX; }
    return result;
}



/* ---------- params ------------ */


int32_t
app_params_first_int32(app_t* msg) {
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT32_MAX; }
    return string_to_int32( app_params_first(msg) );
}

int64_t
app_params_first_int64(app_t* msg) {
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT64_MAX; }
    return string_to_int64( app_params_first(msg) );
}

uint32_t
app_params_first_uint32(app_t* msg) {
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT32_MAX; }
    return string_to_uint32( app_params_first(msg) );
}

uint64_t
app_params_first_uint64(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT64_MAX; }
    return string_to_uint64( app_params_first(msg) );
}

int32_t
app_params_next_int32(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT32_MAX; }
    return string_to_int32( app_params_next(msg) );
}

int64_t
app_params_next_int64(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT64_MAX; }
    return string_to_int64( app_params_next(msg) );
}

uint32_t
app_params_next_uint32(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT32_MAX; }
    return string_to_uint32( app_params_next(msg) );
}

uint64_t
app_params_next_uint64(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT64_MAX; }
    return string_to_uint64( app_params_next(msg) );
}

void
app_params_append_string(app_t* msg, const char *value)
{
    if(!msg) { errno = EINVAL; return ; };

    zlist_t *list = app_get_params(msg);
    if(list == NULL) {
        list = zlist_new();
        zlist_autofree(list);
    }
    zlist_append( list, (void *)value );
    app_set_params( msg, &list );
}

void
app_params_append_int64(app_t* msg, int64_t value )
{
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIi64, value );
    app_params_append_string( msg, buff );
}

void
app_params_append_uint64(app_t* msg, uint64_t value )
{
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIu64, value );
    app_params_append_string( msg, buff );
}


void
app_params_append_int32(app_t* msg, int32_t value )
{
    app_params_append_int64( msg, value );
}

void
app_params_append_uint32(app_t* msg, uint32_t value )
{
    app_params_append_uint64( msg, value );
}


/* -------- args ------------ */
void
app_args_set_string(app_t* msg, const char *key, const char *value) {
    errno = 0;
    if(!msg) { errno = EINVAL; return ; };

    zhash_t *hash = app_get_args(msg);
    if(hash == NULL) {
        hash = zhash_new();
        zhash_autofree(hash);
    }
    zhash_update(hash, key, (void *)value);
    app_set_args(msg, &hash);
}

void
app_args_set_int64(app_t* msg, const char *key, int64_t value ) {
    errno = 0;
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIi64, value );
    app_args_set_string( msg, key, buff );
}

void
app_args_set_uint64(app_t* msg, const char *key, uint64_t value ) {
    errno = 0;
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIu64, value );
    app_args_set_string( msg, key, buff );
}

int32_t
app_args_int32(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return INT32_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return INT32_MAX;
    }
    return string_to_int32(val);
}

int64_t
app_args_int64(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return INT64_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return INT64_MAX;
    }
    return string_to_int64(val);
}

uint32_t
app_args_uint32(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return UINT32_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return UINT32_MAX;
    }
    return string_to_uint32(val);
}

uint64_t
app_args_uint64(app_t* msg, const char *key) {
    if(!msg) { errno = EBADMSG; return UINT64_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return UINT64_MAX;
    }
    return string_to_uint64(val);
}

