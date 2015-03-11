#!/bin/sh

DIR=$(dirname $0)
DIR=$(realpath $DIR)
INCLUDE=$DIR/../include/bios_agent.h
TEMPLATE=$DIR/bios_agent++.h.in
OUTPUT=$DIR/../include/bios_agent++.h

extract_bios_h(){
awk '
BEGIN{
    FS="[ \t,()]+";
    IGNORE["bios_agent_new"] = 1;
    IGNORE["bios_agent_destroy"] = 1;
}
/^BIOS_EXPORT/{
    type = "";
    for( a = 2; a <= NF; a++ ) {
        type = sprintf("%s%s ", type, $a);
    }
    gsub(/[[:space:]]{2,}/," ",type);
    gsub(/^[[:space:]]+/,"",type);
    gsub(/[[:space:]]+$/,"",type);
}
/^[[:space:]]+bios_agent_.+) *;/{
    /* printf("%s %s\n", type, $0 ); */
    funcName=$2;
    if( funcName in IGNORE ) next;
    funcNameObj=substr($2,12)
    i = 3;
    idx = 0;
    while( i < NF ) {
        ptype = $i
        i++;
        if( ptype == "const" ) {
            ptype = sprintf("%s %s", ptype, $i);
            i++;
        }
        ptypes[idx] = ptype;
        name = $i
        pnames[idx] = $i;
        i++;
        idx++
    }
    printf("    %s %s( ",type, funcNameObj);
    for( a=1; a<idx; a++ ) {
        printf(" %s %s", ptypes[a], pnames[a]);
        if( a != idx - 1) printf(", "); ;
    }
    printf(" ) { ");
    if( type != "void" ) printf("return ");
    if( substr( pnames[0], 0, 2) == "**" ) {
        printf("%s( &_bios_agent",funcName);
    } else {
        printf("%s( _bios_agent",funcName);
    }
    for( a=1; a<idx; a++ ) {
        pname = pnames[a]
        gsub(/^\*+/,"",pname);
        printf(", %s", pname);
    }
    printf(" ); };\n");
    type = ""; 
}
'< $INCLUDE
}

while IFS='' read "line" ; do
    if [ "$line" = "@extract_bios_agent_h@" ] ; then
        extract_bios_h
    else
        echo "$line"
    fi
done <$TEMPLATE >$OUTPUT
