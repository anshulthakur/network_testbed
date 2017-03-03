/** @file tuntap.c
 * 
 * @brief Simple tutorial on Tunnel/Tap interfaces
 * 
 * @author Anshul Thakur
 * 
 * @date 08-Feb-2017
 * 
 */

#include <stdio.h> /* Defines perror*/
#include <unistd.h> /* Defines close() */

#include <sys/socket.h> /* Avoids : field ‘ifru_addr’ has incomplete type
   struct sockaddr ifru_addr; error*/
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h> /* Defines O_RDWR */
#include <sys/ioctl.h>

#include <string.h>

/**
 *
 * @brief Create a tunnel
 * @param dev                    Name of interface (or '\0'. Must have enough
 *                               space to hold interface name if '\0' is passed.
 * @param flags                  interface flags IFF_TUN etc.
 *
 * @return descriptor of interface if successful, else a negative value (-1).
 */
int32_t tun_alloc(char *dev, int32_t flags)
{
  struct ifreq ifr = {0};
  int32_t fd, err;

  char *clonedev = "/dev/net/tun";

  /* Open the clone device */
  if((fd= open(clonedev, O_RDWR))<0)
  {
    goto EXIT_LABEL;
  }
  /* prepare struct ifr */
  ifr.ifr_flags = flags; /* IFF_TUN or IFF_TAP, plus, maybe IFF_NO_PI */

  if(*dev)
  {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  /* Try to create device */
  err= ioctl(fd, TUNSETIFF, (void *)&ifr);
  if(err<0)
  {
    close(fd);
    goto EXIT_LABEL;
  }

  /*
   * If successful, write back the name of interface to variable "dev", so the
   * caller also knows it.
   */
  strcpy(dev, ifr.ifr_name);


EXIT_LABEL:
  return(fd);
}/**< tun_alloc */


/**
 * @brief Main routine
 * @return
 */
int32_t main(void)
{
  char tun_name[IFNAMSIZ];
  char tap_name[IFNAMSIZ];

  char *a_name= NULL;

  int32_t group, user;

  int32_t tunfd, tapfd, unnamed_tap_fd;

  strcpy (tun_name, "tun1");
  //tunfd = tun_alloc(tun_name, IFF_TUN);

  strcpy(tap_name, "tap1");
  tapfd = tun_alloc(tap_name, IFF_TAP);

  a_name = (char *) malloc(IFNAMSIZ);
  a_name[0] = '\0';

  //unnamed_tap_fd = tun_alloc(a_name, IFF_TAP);

  /* set tap interface to persistent */
  if(ioctl(tapfd, TUNSETPERSIST, 0) < 0)
  {
    perror("disabling TUNSETPERSIST");
    goto EXIT_LABEL;
  }

  if((user = geteuid())<0)
  {
    perror("Get User ID failed.");
    goto EXIT_LABEL;
  }

  if((group= getegid()) <0)
  {
    perror("Get Group ID failed");
    goto EXIT_LABEL;
  }

  printf("User ID: %d\t Group ID: %d\n", user, group);

  if(ioctl(tapfd, TUNSETOWNER, user) <0)
  {
    perror("enabling TUNSETOWNER");
    goto EXIT_LABEL;
  }

  if(ioctl(tapfd, TUNSETGROUP, user) <0)
  {
    perror("enabling TUNSETGROUP");
    goto EXIT_LABEL;
  }

EXIT_LABEL:

  return(0);
}

