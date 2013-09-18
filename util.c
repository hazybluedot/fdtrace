#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

int list_fd(pid_t pid) {
  DIR *dp;
  struct dirent *ep;
  char buffer[256];

  sprintf(buffer, "/proc/%jd/fd", pid);
  dp = opendir(buffer);
  if (dp == NULL) {
    perror("opendir");
    return -1;
  } else {
    while ( ep = readdir(dp) ) {
      char buf[1024];
      char link[256];
      int len;
      if (ep->d_name[0] != '.') {
	sprintf(link, "%s/%s", buffer, ep->d_name);
	if ((len = readlink(link, buf, sizeof(buf)-1)) != -1)
	  buf[len] = '\0';
	fprintf(stderr, "%s -> %s\n", ep->d_name, buf);
      }
    }
  }
  return 0;
}
