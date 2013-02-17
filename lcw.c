/* console wrapper: Lua library to wrap the console output of a child process
** Copyright (c) 2013 Reuben Thomas <rrt@sc3d.org>
**
** Based on cw (color wrapper)
** Copyright (C) 2004 v9/fakehalo [v9@fakehalo.us]
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**/

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pty.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <lua.h>
#include <lauxlib.h>
#include "lua52compat.h"
#include "dirname.h"
#include "xalloc.h"
#include "minmax.h"

static bool ext=false;
static pid_t pid_c;

/* Exit with an error. */
noreturn void die(const char *error){
 fprintf(stdout,"cw:exit: %s\n",error);
 exit(1);
}

static void sighandler(int sig){
 if(sig==SIGINT&&pid_c)
  kill(pid_c,SIGINT);
#ifdef SIGCHLD
 else if(sig==SIGCHLD)ext=true;
#endif
 if(sig==SIGPIPE||sig==SIGINT){
  fprintf(stderr,"\x1b[00m");
  exit(0);
 }
}

static void sig_catch(int sig, int flags, void (*handler)(int))
{
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = flags;
  sigemptyset(&sa.sa_mask);
  assert(sigaction(sig, &sa, 0)==0);
}

/* Wrap a child process's I/O line by line. */
int wrap_child(lua_State *L){
 luaL_checktype(L, 1, LUA_TFUNCTION);
 int fds[2],fde[2];
 if(pipe(fds)<0)die("pipe() failed.");
 if(pipe(fde)<0)die("pipe() failed.");
 int master[2],slave[2];
 bool ptys_on=(openpty(&master[0],&slave[0],0,0,0)==0)&&
  (openpty(&master[1],&slave[1],0,0,0)==0);
#ifdef SIGCHLD
 sig_catch(SIGCHLD,SA_NOCLDSTOP,sighandler);
#endif
 sig_catch(SIGPIPE,0,sighandler);
 switch((pid_c=fork())){
  case -1:
   die("fork() error.");
   break;
  case 0:
   /* child process to execute the program. */
   if(dup2((ptys_on?slave[0]:fds[1]),STDOUT_FILENO)<0)
    die("dup2() failed.");
   close(fds[0]);
   close(fds[1]);
   if(dup2((ptys_on?slave[1]:fde[1]),STDERR_FILENO)<0)
    die("dup2() failed.");
   close(fde[0]);
   close(fde[1]);
#ifdef HAVE_SETSID
   setsid();
#endif
   break;
  default:
   /* parent process to filter the program's output. (forwards SIGINT to child) */
   sig_catch(SIGINT,0,sighandler);
   if(ptys_on){
    close(fds[0]);
    close(fde[0]);
    fds[0]=master[0];
    fde[0]=master[1];
   }
   fcntl(fds[0],F_SETFL,O_NONBLOCK);
   fcntl(fde[0],F_SETFL,O_NONBLOCK);
   int fdm=MAX(fds[0],fde[0])+1;
   char *linebuf=NULL,*p=NULL;
   ssize_t size=0;
   for(ssize_t s=0;s>0||!ext;){
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fds[0],&rfds);
    FD_SET(fde[0],&rfds);
    if(select(fdm,&rfds,0,0,0)>=0){
     int fd;
     if(FD_ISSET(fds[0],&rfds))fd=fds[0];
     else if(FD_ISSET(fde[0],&rfds))fd=fde[0];
     else continue;
     char tmp[BUFSIZ];
     while((s=read(fd,tmp,BUFSIZ))>0){
      char *q;
      size_t off=p-linebuf;
      linebuf=xrealloc(linebuf,size+s);
      p=linebuf+off;
      memcpy(linebuf+size,tmp,s);
      size+=s;
      while((q=memmem(p,size-(p-linebuf),"\r\n",2))){
       size_t len=q-p;
       int n=fd==fds[0]?STDOUT_FILENO:STDERR_FILENO;
       lua_pushvalue(L,1);
       lua_pushlstring(L,p,len);
       lua_call(L,1,1);
       const char *text=lua_tolstring(L,-1,&len);
       if(text)dprintf(n,"%.*s\r\n",(int)len,text);
       lua_pop(L,1);
       p=q+2;
       if(p-linebuf>=size){
        /* Whenever we completely empty the buffer, free it, to try to avoid
           using too much memory. */
        free(linebuf);
        p=linebuf=NULL;
        size=0;
        break;
       }
      }
     }
    }
   }
   free(linebuf);
   fflush(stdout);
   fflush(stderr);
   int e=0;
   exit(waitpid(pid_c,&e,WNOHANG)>=0&&WIFEXITED(e)?WEXITSTATUS(e):0);
   break;
 }
 return 0;
}

static int Gcanonicalize_file_name (lua_State *L)
{
 char *t = canonicalize_file_name (luaL_checkstring(L, 1));
 if (t) {
  lua_pushstring(L, t);
  free(t);
 } else
  lua_pushnil(L);
 return 1;
}

static const luaL_Reg R[] =
{
 {"canonicalize_file_name", Gcanonicalize_file_name},
 {"wrap_child",             wrap_child},
 {NULL,	NULL}
};

LUALIB_API int luaopen_consolewrap(lua_State *L)
{
 luaL_register(L, "consolewrap", R);
 return 1;
}
