#ifndef SRC_DB_ASSETS_ASSETDEFS_H_
#define SRC_DB_ASSETS_ASSETDEFS_H_

typedef struct _asset_link
{
    a_elmnt_id_t    src;     //!< id of src element
    a_elmnt_id_t    dest;    //!< id of dest element
    a_lnk_src_out_t src_out; //!< outlet in src
    a_lnk_src_out_t dest_in; //!< inlet in dest
    a_lnk_tp_id_t   type;    //!< link type id
} link_t;

#endif // SRC_DB_ASSETS_ASSETDEFS_H_

