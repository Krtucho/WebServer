#define RIO_BUFSIZE 8192
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<errno.h>
// #include <sys/types.h>
//#include <sys/socket.h>


typedef struct {
    int rio_fd;/* Descriptor for this internal buf */
    int rio_cnt;/* Unread bytes in internal buf */
    char *rio_bufptr;/* Next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE];/* Internal buffer */
    } rio_t;

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

    static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
    {
        int cnt;
        
        while (rp->rio_cnt <= 0) {
            /* Refill if buf is empty */
            rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
            sizeof(rp->rio_buf));
            if (rp->rio_cnt < 0) {
                if (errno != EINTR)/* Interrupted by sig handler return */
                return -1;
                }
                else if (rp->rio_cnt == 0)/* EOF */
                return 0;
                else
                rp->rio_bufptr = rp->rio_buf;/* Reset buffer ptr */
                }
                
                /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
                cnt=n;
                if (rp->rio_cnt < n)
                cnt = rp->rio_cnt;
                memcpy(usrbuf, rp->rio_bufptr, cnt);
                rp->rio_bufptr += cnt;
                rp->rio_cnt -= cnt;
                return cnt;
                }




ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    
    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n')
            break;
            } else if (rc == 0) {
                if (n == 1)
                return 0;/* EOF, no data read */
                else
                break;/* EOF, some data was read */
                } else
                return -1;/* Error */
                }
                *bufp = 0;
                return n;
}



ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0) {
            if (errno == EINTR)/* Interrupted by sig handler return */
            nread = 0;/* Call read() again */
            else
            return -1;/* errno set by read() */
            }
            else if (nread == 0)
            break;/* EOF */
            nleft -= nread;
            bufp += nread;
            }
            return (n - nleft);/* Return >= 0 */
}


int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;
    
    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;/* Check errno for cause of error */
    
    /* Fill in the server’s IP address and port */
    if ((hp = gethostbyname(hostname)) == NULL)
    return -2;/* Check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
    (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);
    
    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
    return -1;
    return clientfd;
    }


    int open_listenfd(int port)
    {
        int listenfd, optval=1;
        
        struct sockaddr_in serveraddr;
        
        /* Create a socket descriptor */
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
        
        /* Eliminates "Address already in use" error from bind */
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
        (const void *)&optval , sizeof(int)) < 0)
        return -1;
        
        /* Listenfd will be an end point for all requests to port16on any IP address for this host */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons((unsigned short)port);
        if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
        
        /* Make it a listening socket ready to accept connection requests */
        if (listen(listenfd, LISTENQ) < 0)
        return -1;
        return listenfd;
        }


        void doit(int fd)
        {
            int is_static;
            struct stat sbuf;
            char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
            char filename[MAXLINE], cgiargs[MAXLINE];
            rio_t rio;
            
            /* Read request line and headers */
            Rio_readinitb(&rio, fd);
            Rio_readlineb(&rio, buf, MAXLINE);
            sscanf(buf, "%s %s %s", method, uri, version);
            if (strcasecmp(method, "GET")) {
                clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
                return;
                }
                read_requesthdrs(&rio);
                
                /* Parse URI from GET request */
                is_static = parse_uri(uri, filename, cgiargs);
                if (stat(filename, &sbuf) < 0) {
                    clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn’t find this file");
                    return;
                    }
                    
                    if (is_static) {/* Serve static content */
                    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
                        clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn’t read the file");
                        return;
                        }
                        serve_static(fd, filename, sbuf.st_size);
                        }
                        else {/* Serve dynamic content */
                        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
                            clienterror(fd, filename, "403", "Forbidden",
                            
                            "Tiny couldn’t run the CGI program");
                            return;
                            }
                            serve_dynamic(fd, filename, cgiargs);
                            }
                            }



int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    
    if (!strstr(uri, "cgi-bin")) {/* Static content */
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri)-1] == ’/’)
    strcat(filename, "home.html");
    return 1;
    }
    else {/* Dynamic content */
    ptr = index(uri, '?');
    if (ptr) {
        strcpy(cgiargs, ptr+1);
        *ptr = '\0';
        }
        else
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
        }
        }


void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    
    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    
    /* Send response body to client */
    
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
    }
    
    /*23* get_filetype - derive file type from file name24*/
    void get_filetype(char *filename, char *filetype)
    {
        if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
        else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
        else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
        else
        strcpy(filetype, "text/plain");
        
        }












