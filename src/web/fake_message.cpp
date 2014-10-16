#include <czmq.h>

#include "asset_msg.h"

int main(int argc, char** argv) {
    char key[1024];
    char value[1024];
    char name[1024];
    int id;
    asset_msg_t *element = asset_msg_new(ASSET_MSG_ELEMENT);

    printf("Enter element id: ");
    scanf("%d", &id);
    printf("Enter element name: ");
    scanf("%s", name);

    asset_msg_set_name(element, "%s", name);
    asset_msg_set_location(element, 0);
    asset_msg_set_type(element, 0);

    sprintf(key, "y");
    sprintf(value, "y");

    while(strcmp(key, "exit") != 0) {
        printf("Enter key in ext: ");
        scanf("%s", key);
        if(strcmp(key, "exit") == 0)
            break;
        printf("Enter value of %s: ", key);
        scanf("%s", value);
        asset_msg_ext_insert(element, key, "%s", value);
    }

    zmsg_t* msg = asset_msg_encode_return_element(id, asset_msg_encode(&element));
    sprintf(key, "%d", id);
    FILE* fl = fopen(key, "w");
    zmsg_save(msg, fl);
    fclose(fl);
    return 0;
}
