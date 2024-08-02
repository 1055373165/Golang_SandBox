/*
* tiny.c - A simple, iterative HTTP1.0 Web server that 
* uses the GET method to serve static and dynamic content
*/
# include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *cgiargs);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

/*
    * 监听在命令行中传递来的端口上的连接请求；在通过调用 open_listenfd 函数打开一个监听套接字以后；
    * TINY 执行典型的无限服务器循环，不断地接收连接请求、执行事务处理、并在服务完成后关闭连接套接字。
*/
int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    // 用于接收客户端请求，并将客户端的套接字地址填充进 clientaddr 中；服务器查看该结构即可获悉通信客户端信息
    struct sockaddr_storage clientaddr;

    /* Check command-line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // socket fd (active fd)
    listenfd = Open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        // clientaddr type 强制转换为 socket_addr 这种通用类型，消除协议差异性
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // 当 Accept 返回时， clientaddr 中已经填充了请求连接客户端的地址信息
        
        // 从 clientaddr 获取客户端的 host（域名 or ip） 和 service（port 或服务协议名如 http） 信息
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
}   

/*
    * doit 函数处理一个 http 请求
    * 1. 首先读和解析请求行
    * 2. 将 URI 解析为一个文件名和一个可能为空的 CGI 参数字符串，并设置请求的事静态还是动态内容的标志
    * 3. 如果请求的是静态内容，验证该文件是一个普通文件并且拥有操作权限；如果请求的是动态内容，验证该文件是一个可执行文件，并返回执行结果。
*/
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd); // 将 fd 和缓冲区绑定
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers: \n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not Found", "Tiny couldn't find this file");
        return;
    }

    if (is_static) { /* Serve static content */ 
        // 检查是否为普通文件 检查是否拥有读权限
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR &sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else { /* Server dynamic content */
        // 可执行文件 拥有执行权限
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR &sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}


/*
clienterror 函数发送一个 http 响应到客户端，在响应行中包含响应的状态码和状态消息，响应主体中包含一个 HTML 文件，向浏览器用户解释错误
*/
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the http response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the http response */
    sprintf(buf, "HTTP/1.0 %s %s \r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    // write response header 
    Rio_writen(fd, buf, strlen(buf));
    // write response body
    Rio_writen(fd, body, strlen(body));
}

/*
read_requesthdrs 函数读取并忽略请求头或者响应头
*/
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

/*
* parse_uri 将 URI 解析为一个文件名和一个可选地 CGI 参数字符串；
* 如果请求的是静态内容的话，那么清除 CGI 参数字符串，然后将 URI 转换为一个 Linux 相对路径名（比如 ./index.html)，而如果 URI 是使用 "/" 结尾的，把默认的文件名添加到后面。
* 如果请求的是动态内容的话，提取出所有的 CGI 参数用于函数调用（可执行文件的主目录为 ./cgi-bin
*/
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) { /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri)-1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else { /* Dynamic content */
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

/*
    serve_static: tiny 提供五种常见类型的静态内容：HTML 文件、无格式的文本文件、编码为 GIF PNG JPG 格式的图片
    serve_static 函数发送一个 http 响应，其 body 包含一个本地文件的内容。
    首先通过检查文件名的后缀来判断文件类型，并且发送响应行和响应报头给客户端。
*/
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP1/.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers: \n");
    printf("%s", buf);

    /* Send response body to client */
    // 源文件对应的文件描述符
    srcfd = Open(filename, O_RDONLY, 0);
    // 将 srcfd 内容映射到虚拟内存空间并返回这个内存空间的指针
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    // 将虚拟内存空间中 filesize 大小的内容写入 fd 中返回给客户端
    Rio_writen(fd, srcp, filesize);
    // 取消内存映射
    Munmap(srcp, filesize);
}
/*
    将被请求文件的内容复制到已连接描述符 fd 来发送响应主体。
    1. 先以只读方式打开 filename 对应的文件，并获得它的描述符；
    2. Linux mmap 函数将被请求文件映射到一个虚拟内存空间（调用 mmap 将文件 srcfd 的前 filesize 个字节映射到一个从地址 srcp 开始的私有只读虚拟内存区域
    3. 一旦将文件映射到内存，就不再需要它的文件描述符了，所以可以提前关闭这个文件。
    4. rio_writen 函数复制从 srcp 未知开始的 filesize 个字节到客户端的已连接文件描述符。
    5. 最后释放了映射的虚拟内存区域
*/

/* get_filetype - Derive file type from filename */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
        strcpy(filetype, ".html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "iamge/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpg");
    else if (strstr(filename, ".jpeg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* return first part of HTTP response */
    // 使用 sprintf 构造好响应首行到字节缓冲区 buf 中
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));

    // 响应首部
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0) { /* child process */
        /* Real Server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */
        Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child 并发进程控制*/
}
/*
    派生出一个新的子进程，子进程用来自请求 URI 的 CGI 参数初始化 QUERY_STRING 环境变量。
    子进程重定向它的标准输出到已连接文件描述符，然后加载并运行 CGI 程序。
    因为 CGI 程序运行在子进程的上下文中，它能够访问所有在调用 execve 函数之前就存在的打开文件和环境变量。
    因此 CGI 程序写到标准输出上的任何东西都将直接送到客户端进程，不会收到任何来自父进程的干涉。
    其间，父进程阻塞在对 wait 的调用中，等待当紫禁城终止时，回收操作系统分配给子进程的资源。
*/