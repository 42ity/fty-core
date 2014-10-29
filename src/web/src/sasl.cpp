#include <sasl/sasl.h>
#include <stdio.h>

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>

// TODO: move to some common header
// https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define xstr(a) str(a)
#define str(a) #a

#ifndef SASLAUTHD_MUX
#error Build with -DSASLAUTHD_MUX=/path/to/saslauthd/mux
#else
#define SASLAUTHD_MUX_PATH xstr(SASLAUTHD_MUX)
#endif

/**************************************************************
 * I/O wrapper to attempt to write out the specified vector.
 * data, without any guarantees. If the function returns
 * -1, the vector wasn't completely written.
 **************************************************************/
int retry_writev(int fd, struct iovec *iov, int iovcnt) {
    int n;               /* return value from writev() */
    int i;               /* loop counter */
    int written;         /* bytes written so far */
    static int iov_max;  /* max number of iovec entries */

#ifdef MAXIOV
    iov_max = MAXIOV;
#else
# ifdef IOV_MAX
    iov_max = IOV_MAX;
# else
    iov_max = 8192;
# endif
#endif

    written = 0;

    for (;;) {

        while (iovcnt && iov[0].iov_len == 0) {
            iov++;
            iovcnt--;
        }

        if (!iovcnt) {
            return written;
        }

        n = writev(fd, iov, iovcnt > iov_max ? iov_max : iovcnt);

        if (n == -1) {
            if (errno == EINVAL && iov_max > 10) {
                iov_max /= 2;
                continue;
            }

            if (errno == EINTR) {
                continue;
            }

            return -1;

        } else {
            written += n;
        }

        for (i = 0; i < iovcnt; i++) {
            if ((int) iov[i].iov_len > n) {
                iov[i].iov_base = (char *)iov[i].iov_base + n;
                iov[i].iov_len -= n;
                break;
            }

            n -= iov[i].iov_len;
            iov[i].iov_len = 0;
        }

        if (i == iovcnt) {
            return written;
        }
    }
}


/*
 * Keep calling the read() system call with 'fd', 'buf', and 'nbyte'
 * until all the data is read in or an error occurs.
 */
int retry_read(int fd, void *inbuf, unsigned nbyte) {
    int n;
    int nread = 0;
    char *buf = (char *)inbuf;

    if (nbyte == 0) {
        return 0;
    }

    for (;;) {
        n = read(fd, buf, nbyte);
        if (n == -1 || n == 0) {
            if (errno == EINTR || errno == EAGAIN) {
                continue;
            }
            return -1;
        }

        nread += n;

        if (n >= (int) nbyte) {
            return nread;
        }

        buf += n;
        nbyte -= n;
    }
}

bool authenticate(const char *userid, const char *passwd, const char *service) {
    char response[1024];
    char query[8192];
    char *query_end = query;
    int s;
    struct sockaddr_un srvaddr;
    int r;
    unsigned short count;
    void *context;
    char pwpath[sizeof(srvaddr.sun_path)];
    const char *p = NULL;

    if (!service) {
        service = "imap";
    }
    if (!userid || !passwd) {
        return false;
    }

    strcpy(pwpath, SASLAUTHD_MUX_PATH);

    /*
     * build request of the form:
     *
     * count authid count password count service count realm
     */
    unsigned short u_len, p_len, s_len, r_len;
    struct iovec iov[8];

    u_len = htons(strlen(userid));
    p_len = htons(strlen(passwd));
    s_len = htons(strlen(service));
    r_len = htons(0);

    memcpy(query_end, &u_len, sizeof(unsigned short));
    query_end += sizeof(unsigned short);
    while (*userid) {
        *query_end++ = *userid++;
    }

    memcpy(query_end, &p_len, sizeof(unsigned short));
    query_end += sizeof(unsigned short);
    while (*passwd) {
        *query_end++ = *passwd++;
    }

    memcpy(query_end, &s_len, sizeof(unsigned short));
    query_end += sizeof(unsigned short);
    while (*service) {
        *query_end++ = *service++;
    }

    memcpy(query_end, &r_len, sizeof(unsigned short));
    query_end += sizeof(unsigned short);

    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s == -1) {
        throw std::runtime_error("Can't create SASL socket!");
        return false;
    }

    memset((char *)&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sun_family = AF_UNIX;
    strncpy(srvaddr.sun_path, pwpath, sizeof(srvaddr.sun_path));

    r = connect(s, (struct sockaddr *) &srvaddr, sizeof(srvaddr));
    if (r == -1) {
        throw std::runtime_error("Can't connect to SASL!");
        return false;
    }

    {

        struct iovec iov[8];

        iov[0].iov_len = query_end - query;
        iov[0].iov_base = query;

        if (retry_writev(s, iov, 1) == -1) {
            fprintf(stderr, "write failed\n");
            return false;
        }

        /*
         * read response of the form:
         *
         * count result
         */
        if (retry_read(s, &count, sizeof(count)) < (int) sizeof(count)) {
            fprintf(stderr, "size read failed\n");
            return false;
        }

        count = ntohs(count);
        if (count < 2) { /* MUST have at least "OK./saslauthd/testsaslauthd.c" or "NO" */
            close(s);
            fprintf(stderr, "bad response from saslauthd\n");
            return false;
        }

        count = (int)sizeof(response) < count ? sizeof(response) : count;
        if (retry_read(s, response, count) < count) {
            close(s);
            fprintf(stderr, "read failed\n");
            return false;
        }
        response[count] = '\0';

        close(s);

        if (!strncmp(response, "OK", 2)) {
            return true;
        }
        fprintf(stderr, "saslauthd authentication failed: '%s'\n", response);

    }
    return false;
}
