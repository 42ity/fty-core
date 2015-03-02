#include <czmq.h>
#include <bios_agent.h>
#include <stdio.h>

int main() {

    int r;

    static const char* endpoint = "ipc://@/malamute";
    static const char* content_type = "application/octet-stream";

    bios_agent_t *reader = bios_agent_new(endpoint, "reader");
    assert(reader);
    fprintf(stderr, ">>>>>> reader::new done\n");
    
    bios_agent_t *writer = bios_agent_new(endpoint, "writer");
    assert(writer);
    fprintf(stderr, ">>>>>> writer::new done\n");

    {
    ymsg_t *ymsg = ymsg_new(YMSG_SEND);
    ymsg_set_content_type(ymsg, content_type);

    fprintf(stderr, ">>>>>> writer::sendto starting\n");
    r = bios_agent_sendto(writer, "reader", "test-subject", &ymsg);
    fprintf(stderr, ">>>>>> writer::sendto done, r:%d\n", r);
    assert(r == 0);
    }

    {
    fprintf(stderr, ">>>>>> writer::recv starting\n");
    ymsg_t *ymsg = bios_agent_recv(reader);
    fprintf(stderr, ">>>>>> writer::reader done\n");
    const char* mime = ymsg_content_type(ymsg);
    assert(streq(mime, content_type));
    ymsg_destroy(&ymsg);
    }

    bios_agent_destroy(&writer);
    bios_agent_destroy(&reader);

}
