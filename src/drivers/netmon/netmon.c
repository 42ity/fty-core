/* netmon.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

/*
Description:
    Retrieved from: https://github.com/infoburp/iproute2.git

Adaptation by: Michal Vyskocil <michalvyskocil@eaton.com>,
               Karol Hrdina <karolhrdina@eaton.com>

References: BIOS-247, BIOS-406

TODO: Resolve ctrl-c unresponsiveness
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <asm/types.h>
#include <libnetlink.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/netdevice.h>

#include <czmq.h>

#include "log.h"
#include "defs.h"
#include "msg_send.h"

// include/utils.h
#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]
#define DN_MAXADDL 20

// ip/ip_utils.h
#ifndef	INFINITY_LIFE_TIME
#define     INFINITY_LIFE_TIME      0xFFFFFFFFU
#endif

// ip/ipaddr.c
struct rtnl_handle rth = { .fd = -1 };
#define _SL_ "\n"

// ip/ipaddress.c
// by GDB
static int filter_family = 0;
// ip a s scope link
static int filter_scope = 253;
static int filter_scopemask = -1;
static int preferred_family = 0;

// wheter to use zmq or print to stdout
static bool use_zmq = false;

// include/utils.h
struct dn_naddr
{
    unsigned short          a_len;
    unsigned char a_addr[DN_MAXADDL];
};

// ip/ipaddress.c
struct nlmsg_list
{
	struct nlmsg_list *next;
	struct nlmsghdr	  h;
};

// ip/ipaddress.c
struct nlmsg_chain
{
	struct nlmsg_list *head;
	struct nlmsg_list *tail;
};

// lib/ll_map.c
const char *ll_index_to_name(unsigned idx);

// include/rt_names.h
const char *ll_addr_n2a(unsigned char *addr, int alen,
			int type, char *buf, int blen);

// lib/ll_addr.c
const char *ll_addr_n2a(unsigned char *addr, int alen, int type, char *buf, int blen)
{
	int i;
	int l;

	if (alen == 4 &&
	    (type == ARPHRD_TUNNEL || type == ARPHRD_SIT || type == ARPHRD_IPGRE)) {
		return inet_ntop(AF_INET, addr, buf, blen);
	}
	if (alen == 16 && type == ARPHRD_TUNNEL6) {
		return inet_ntop(AF_INET6, addr, buf, blen);
	}
	l = 0;
	for (i=0; i<alen; i++) {
		if (i==0) {
			snprintf(buf+l, blen, "%02x", addr[i]);
			blen -= 2;
			l += 2;
		} else {
			snprintf(buf+l, blen, ":%02x", addr[i]);
			blen -= 3;
			l += 3;
		}
	}
	return buf;
}

// include/utils.h
static inline __u32 nl_mgrp(__u32 group)
{
	if (group > 31 ) {
		fprintf(stderr, "Use setsockopt for this group %d\n", group);
		exit(-1);
	}
	return group ? (1 << (group - 1)) : 0;
}

// ip/ipaddress.c
static unsigned int get_ifa_flags(struct ifaddrmsg *ifa,
				  struct rtattr *ifa_flags_attr)
{
	return ifa_flags_attr ? rta_getattr_u32(ifa_flags_attr) :
				ifa->ifa_flags;
}

// lib/utils.c
const char *rt_addr_n2a(int af, int len, const void *addr, char *buf, int buflen)
{
	switch (af) {
	case AF_INET:
	case AF_INET6:
		return inet_ntop(af, addr, buf, buflen);
	default:
		return "???";
	}
}

// lib/utils.c
const char *format_host(int af, int len, const void *addr,
			char *buf, int buflen)
{
	return rt_addr_n2a(af, len, addr, buf, buflen);
}

// MVY: quick and dirty mac address resolution - as I've kicked out most of the code dealing
//      with link, this is a code returns the mac for given ethname from /sysfs
//      not thread safe, never ever use or copy this code!!!
const char *qd_mac(const char* ethname) {
    static char mac[MAX_ADDR_LEN];
    char *path;
    ssize_t r;
    int fd;
    
    r = asprintf(&path, "/sys/class/net/%s/address", ethname);
    if (!r) {
        log_error("can't allocate path string: %m\n");
        return "";
    }

    fd = open(path, O_RDONLY);
    if (!fd) {
        log_error("can't open /sys/class/net/%s/address: %m\n", ethname);
        free(path);
        return "";
    }

    r = read(fd, mac, MAX_ADDR_LEN);
    if (r < 1) {
        log_error("read on %s failed: %m\n", path);
        free(path);
        close(fd);
        return "";
    }
    mac[r-1] = '\0'; // kill \n
    free(path);
    close(fd);
    return (const char*) mac;
}

static int
print_addrinfo (const struct sockaddr_nl *who, struct nlmsghdr *n,
                void *requester) {    
    int ret = -1;

    struct ifaddrmsg *ifa = NLMSG_DATA(n);
	//struct ifinfomsg *ifi = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	int deprecated = 0;
	/* Use local copy of ifa_flags to not interfere with filtering code */
	unsigned int ifa_flags;
	struct rtattr * rta_tb[IFA_MAX+1];
	char abuf[256];
	SPRINT_BUF(b1);

    const char *ethname = ll_index_to_name(ifa->ifa_index);
    uint8_t ipfamily;
    const char *ipaddress = NULL;
    uint8_t prefixlen = 0;
    const char *mac = qd_mac(ethname);
	
    // sanity check
    if (n->nlmsg_type != RTM_NEWADDR && n->nlmsg_type != RTM_DELADDR)
		return 0;

    const char *type = n->nlmsg_type == RTM_NEWADDR ?
        NETMON_EVENT_ADD : NETMON_EVENT_DEL;
	
    len -= NLMSG_LENGTH(sizeof(*ifa));
	if (len < 0) {
		log_error("wrong nlmsg len %d\n", len);
		return -1;
	}

    switch (ifa->ifa_family) {
        case AF_INET:
            ipfamily = NETDISC_IPVER_IPV4; // inet
            break;
        case AF_INET6:
            ipfamily = NETDISC_IPVER_IPV6; //inet6
            break;
        default:
            log_warning("unsupported family: %d\n", ifa->ifa_family);
            return 0;
    }
    
    //this is needed to have correct ip addresses...
    parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa),
		     n->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa)));

    if (!rta_tb[IFA_LOCAL])
		rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];
	if (!rta_tb[IFA_ADDRESS])
		rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];
	
	if (rta_tb[IFA_LOCAL]) {
		ipaddress = format_host(ifa->ifa_family,
					      RTA_PAYLOAD(rta_tb[IFA_LOCAL]),
					      RTA_DATA(rta_tb[IFA_LOCAL]),
					      abuf, sizeof(abuf));

        
		if (rta_tb[IFA_ADDRESS] == NULL ||
		    memcmp(RTA_DATA(rta_tb[IFA_ADDRESS]), RTA_DATA(rta_tb[IFA_LOCAL]),
			   ifa->ifa_family == AF_INET ? 4 : 16) == 0) {
            prefixlen = ifa->ifa_prefixlen;
        }
	}
		
    /* XXX: following code does not work! The problem is in ifa/ifi above, so we have qd_mac function!
    if (rta_tb[IFLA_ADDRESS]) {
			mac = ll_addr_n2a(RTA_DATA(rta_tb[IFLA_ADDRESS]),
						      RTA_PAYLOAD(rta_tb[IFLA_ADDRESS]),
						      ifi->ifi_type,
						      abuf, sizeof(abuf));
    }
    */
    
    netmon_msg_send (type, ethname, ipfamily, ipaddress, prefixlen, mac, requester);
	return 0;
}


static int store_nlmsg(const struct sockaddr_nl *who, struct nlmsghdr *n,
		       void *arg)
{
	struct nlmsg_chain *lchain = (struct nlmsg_chain *)arg;
	struct nlmsg_list *h;

	h = malloc(n->nlmsg_len+sizeof(void*));
	if (h == NULL)
		return -1;

	memcpy(&h->h, n, n->nlmsg_len);
	h->next = NULL;

	if (lchain->tail)
		lchain->tail->next = h;
	else
		lchain->head = h;
	lchain->tail = h;

	ll_remember_index(who, n, NULL);
	return 0;
}

static void ipaddr_filter(struct nlmsg_chain *linfo, struct nlmsg_chain *ainfo)
{
	struct nlmsg_list *l, **lp;

	lp = &linfo->head;
	while ( (l = *lp) != NULL) {
		int ok = 0;
		int missing_net_address = 1;
		struct ifinfomsg *ifi = NLMSG_DATA(&l->h);
		struct nlmsg_list *a;

		for (a = ainfo->head; a; a = a->next) {
			struct nlmsghdr *n = &a->h;
			struct ifaddrmsg *ifa = NLMSG_DATA(n);
			struct rtattr *tb[IFA_MAX + 1];
			unsigned int ifa_flags;

			if (ifa->ifa_index != ifi->ifi_index)
				continue;
			missing_net_address = 0;
			if (filter_family && filter_family != ifa->ifa_family)
				continue;
			if ((filter_scope^ifa->ifa_scope)&filter_scopemask)
				continue;

			parse_rtattr(tb, IFA_MAX, IFA_RTA(ifa), IFA_PAYLOAD(n));
			ifa_flags = get_ifa_flags(ifa, tb[IFA_FLAGS]);

			ok = 1;
			break;
		}
		if (missing_net_address &&
		    (filter_family == AF_UNSPEC || filter_family == AF_PACKET))
			ok = 1;
		if (!ok) {
			*lp = l->next;
			free(l);
		} else
			lp = &l->next;
	}
}

static void free_nlmsg_chain(struct nlmsg_chain *info)
{
	struct nlmsg_list *l, *n;

	for (l = info->head; l; l = n) {
		n = l->next;
		free(l);
	}
}

static int print_selected_addrinfo(int ifindex, struct nlmsg_list *ainfo, void *requester)
{
	for ( ;ainfo ;  ainfo = ainfo->next) {
		struct nlmsghdr *n = &ainfo->h;
		struct ifaddrmsg *ifa = NLMSG_DATA(n);

		if (n->nlmsg_type != RTM_NEWADDR)
			continue;

		if (n->nlmsg_len < NLMSG_LENGTH(sizeof(ifa)))
			return -1;

		if (ifa->ifa_index != ifindex) {
		    //(filter.family && filter.family != ifa->ifa_family))
			continue;
        }

		print_addrinfo(NULL, n, requester);
	}
	return 0;
}

/*
 * print the info about network interfaces in form
 * (name, type, network, mac)
 *
 * (enp0s25, inet, 10.130.38.187/24, a0:1d:48:b7:e2:4e)
 * (enp0s25, inet6, fe80::a21d:48ff:feb7:e24e/64, a0:1d:48:b7:e2:4e)
 *
 * BTW: the printing is done in callback print_selected_addrinfo/print_addrinfo
 */
static int ipaddr_list(void* requester)
{
	struct nlmsg_chain linfo = { NULL, NULL};
	struct nlmsg_chain ainfo = { NULL, NULL};
	struct nlmsg_list *l;
	char *filter_dev = NULL;
	int no_link = 0;

	if (rtnl_wilddump_request(&rth, preferred_family, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(&rth, store_nlmsg, &linfo) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

	if (filter_family != AF_PACKET) {

		if (rtnl_wilddump_request(&rth, filter_family, RTM_GETADDR) < 0) {
			perror("Cannot send dump request");
			exit(1);
		}

		if (rtnl_dump_filter(&rth, store_nlmsg, &ainfo) < 0) {
			fprintf(stderr, "Dump terminated\n");
			exit(1);
		}

		ipaddr_filter(&linfo, &ainfo);
	}

	for (l = linfo.head; l; l = l->next) {
        struct ifinfomsg *ifi = NLMSG_DATA(&l->h);
        if (filter_family != AF_PACKET)
            print_selected_addrinfo(ifi->ifi_index,
                        ainfo.head, requester);
	}
	free_nlmsg_chain(&ainfo);
	free_nlmsg_chain(&linfo);

	return 0;
}

static int accept_msg(const struct sockaddr_nl *who,
		      struct nlmsghdr *n, void *requester)
{
    switch (n->nlmsg_type) {

        case RTM_NEWADDR:
        case RTM_DELADDR:
            print_addrinfo(who, n, requester);
            break;

        case RTM_NEWROUTE:
        case RTM_DELROUTE:
        case RTM_NEWLINK:
        case RTM_DELLINK:
        case RTM_NEWADDRLABEL:
        case RTM_DELADDRLABEL:
        case RTM_NEWNEIGH:
        case RTM_DELNEIGH:
        case RTM_NEWPREFIX:
        case RTM_NEWRULE:
        case RTM_DELRULE:
        case RTM_NEWNETCONF:
        case 15:
        case RTM_NEWQDISC:
        case RTM_DELQDISC:
        case RTM_NEWTCLASS:
        case RTM_DELTCLASS:
        case RTM_NEWTFILTER:
        case RTM_DELTFILTER:
        case NLMSG_ERROR:
        case NLMSG_NOOP:
        case NLMSG_DONE:
            break;
        default:
            log_error("n->nlmsg_type = <default>\n");
            break;
    }
    return 0;
}

int main(int argc, char **argv) {

    if (isatty(STDERR_FILENO)) {
        fprintf(stderr, "%s", "WARNING: netmon does communicate through zeromq bus, so does not prints anything to stdout\n");
        fprintf(stderr, "%s", "         Please start simple, which will autospawn netmon internally\n");
        fprintf(stderr, "%s", "WARNING: correct SIGTERM handling (CTRL+C) is not yet implemented, use kill -9 for now\n");
    }

//    zsys_catch_interrupts ();
//    zsys_handler_set (&interrupt);

    unsigned groups = ~RTMGRP_TC;
	const char *prog = *argv; 

    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);

    rtnl_close(&rth);
    argc--;	argv++;

    groups |= nl_mgrp(RTNLGRP_IPV4_IFADDR);
    groups |= nl_mgrp(RTNLGRP_IPV6_IFADDR);

	if (rtnl_open(&rth, groups) < 0)
		exit(1);

    ipaddr_list(dbsock);
	if (rtnl_listen(&rth, accept_msg, dbsock) < 0)
		exit(2);

    zsock_destroy(&dbsock);
    assert (dbsock == NULL);
	
	return 0;
}
