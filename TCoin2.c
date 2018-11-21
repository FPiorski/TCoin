#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include <ctype.h>

int conn;
char sbuf[512];

int raw(char *fmt, ...);
int crypto(char *msg, char *res);

FILE *logfp, *fp;

int main(void){
    char *logfile="bot.log";
    char *nick="TCoin";
    char *channel="#absolutelyuniquetestchannel";
    char *host="irc.freenode.net";
    char *port="6667";
    
    char *user, *command, *where, *message, *sep, *target;
    
    int i, j, l, sl, o = -1, start, wordcount, joined = 0;
    char buf[513], response[513];
    struct addrinfo hints, *res;

    logfp=fopen(logfile, "a");

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, &hints, &res);
    conn = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(conn, res->ai_addr, res->ai_addrlen);
    
    raw("USER %s 0 0 :%s\r\n", nick, nick);
    raw("NICK %s\r\n", nick);
    
    while ((sl = read(conn, sbuf, 512))) {
        for (i=0; i<sl; ++i){
            o++;
            buf[o] = sbuf[i];
            if ((i > 0 && sbuf[i] == '\n' && sbuf[i-1] == '\r') || o == 512) {
                buf[o+1] = '\0';
                l = o;
                o = -1;
                
                printf(">> %s", buf);
                if (joined)
                    fprintf(logfp, ">> %s", buf);
                
                if (!strncmp(buf, "PING", 4)) {
                    buf[1]='O';
                    raw(buf);
                } else if (buf[0] == ':') {
                    wordcount = 0;
                    user = command = where = message = NULL;
                    for (j=1; j<l; ++j) {
                        if (buf[j] == ' ') {
                            buf[j]='\0';
                            wordcount++;
                            switch(wordcount) {
                                case 1: user = buf + 1; break;
                                case 2: command = buf + start; break;
                                case 3: where = buf + start; break;
                            }
                            if (j == l-1) continue;
                            start = j+1;
                        } else if (buf[j] == ':' && wordcount == 3) {
                            if (j < l-1) message = buf+j+1;
                            break;
                        }
                    }
                    
                    if (wordcount < 2) continue;
                    
                    if (!strncmp(command, "001", 3) && channel != NULL) {
                        raw("JOIN %s\r\n", channel);
                    } else if (!strncmp(command, "PRIVMSG", 7) || !strncmp(command, "NOTICE", 6)) {
                        if (where==NULL || message==NULL) continue;
                        if ((sep=strchr(user, '!')) != NULL) user[sep-user] = '\0';
                        if (where[0]=='#' || where[0]=='&' || where[0]=='+' || where[0]=='!') target = where; else target = user;
                        printf("[from: %s] [reply-with: %s] [where: %s] [reply-to: %s] %s", user, command, where, target, message);
                        if (message[0]=='!'){
                            joined = 1;
                            
                            message++;
                            response[0]='\0';
                            
                            int flag=0;
                            
                            if (!strncmp(message, "command1", 12))
                                ;
                            else if (!strncmp(message, "command2", 12))
                                ;
                            else if (!strncmp(message, "command2", 12))
                                ;
                            else
                                flag = crypto(message, response);

                            if (!flag)
                                raw("%s %s :%s: %s", command, target, user, response);
                            }
                    }
                }
                
            }
        }
        
    }

    fclose(logfp);

    return 0;
    
}

int raw(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sbuf, 512, fmt, ap);
    va_end(ap);
    write(conn, sbuf, strlen(sbuf));
    fprintf(logfp, "<< %s", sbuf);
    return printf("<< %s", sbuf) - 3;
}

int crypto(char *msg, char *res){
    char ccur[100], rcur[100], t1[100], syscall[1000];
    double usdval, btcval, rcurval;
    int l=0, bo=0;
    while ( !isspace(msg[l]) && msg[l]!='\0' && msg[l]!='\r' && msg[l]!='\n' && l<99 ){
        ccur[l] = msg[l];
        ++l;
    }
    ccur[l]='\0';
    while ( isspace(msg[l]) && msg[l]!='\0' && msg[l]!='\r' && msg[l]!='\n' )
        ++l;
    msg+=l;
    l=0;
    while ( !isspace(msg[l]) && msg[l]!='\0' && msg[l]!='\r' && msg[l]!='\n' && l<99 ){
        rcur[l] = msg[l];
        ++l;
    }
    rcur[l] = '\0';

    int snexit=0;
    while(ccur[snexit]!='\0'){
        ccur[snexit] = tolower(ccur[snexit]);
        ++snexit;
    }
    if ( (strlen(ccur) != 3) && (strncmp(ccur, "ioteth", 6)) )
        return 1;

    strcpy(syscall, "curl -s https://api.bitfinex.com/v1/pubticker/");
    strcat(syscall, ccur);
    if (strncmp(ccur, "ioteth", 6))
        strcat(syscall, "usd");
    strcat(syscall, " | grep -Po '\"mid\":.*?[^\\\\]\",' | tr \",\" \"\\n\" | sed 's/\"//g;s/mid://' | tr -d \"\\n\"");
    printf("\n\n%s\n\n", syscall);
    fp=popen(syscall, "r");
    t1[0] = '\0';
    if (fp != NULL){
        fgets(t1, 1000, fp);
        if (t1[0] == '\0')
            return 2;
    }else
        return 3;
    pclose(fp);
    sscanf(t1, "%lf", &usdval);

    if (strncmp(ccur, "btc", 3) && strncmp(ccur, "ioteth", 6)){       
        strcpy(syscall, "curl -s https://api.bitfinex.com/v1/pubticker/");
        strcat(syscall, ccur);
        strcat(syscall, "btc | grep -Po '\"mid\":.*?[^\\\\]\",' | tr \",\" \"\\n\" | sed 's/\"//g;s/mid://' | tr -d \"\\n\"");
        fp=popen(syscall, "r");
        t1[0] = '\0';
        if (fp != NULL){
            fgets(t1, 1000, fp);
            if (t1[0] == '\0')
                return 4;
        }else
            return 5;
        pclose(fp);
        sscanf(t1, "%lf", &btcval);
    }

    if (strlen(rcur)==3){
        strcpy(syscall, "curl -s http://api.fixer.io/latest?base=USD | sed 's/\",\"/\\n/g' | grep rates | tr \",\" \"\\n\" | tr \"{\" \"\\n\" | tr \"}\" \"\\n\" | grep -i ");
        strcat(syscall, rcur);
        strcat(syscall, " | tr \":\" \"\\n\" | grep \"\\.\"");
        fp=popen(syscall, "r");
        t1[0] = '\0';
        if (fp != NULL){
            fgets(t1, 1000, fp);
            if (t1[0] == '\0')
                return 6;
        }else
            return 7;
        pclose(fp);
        sscanf(t1, "%lf", &rcurval);
        bo=1;
    }

    strcpy(res, "Current ");
    for (int i=0; i<3; ++i)
        t1[i] = toupper(ccur[i]);
    t1[3]='\0';
    strcat(res, t1); 
    strcat(res, " Prices: \x03" "02 ");
    if (rcur[0] == '\0' || !bo){
        if (strncmp(ccur, "ioteth", 6)){
            strcat(res, "USD: ");
            sprintf(t1, "%.2lf", usdval);
        }else{
            strcat(res, "ETH: ");
            sprintf(t1, "%.10lf", usdval);
        }
    }else{
        for (int i=0; i<4; ++i)
            t1[i] = toupper(rcur[i]);
        strcat(res, t1);
        strcat(res, ": ");
        sprintf(t1, "%.2lf", usdval * rcurval);
    }
    strcat(res, t1);
    if (strncmp(ccur, "btc", 3) && strncmp(ccur, "ioteth", 6)){
        sprintf(t1, "%.7lf", btcval);
        strcat(res, " \x03" "13BTC: ");
    }else
        t1[0]='\0';
    strcat(res, t1);
    strcat(res, "\r\n");

    return 0;
}
