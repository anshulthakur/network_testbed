/*
 * main.c
 *
 *  Created on: 01-Mar-2017
 *      Author: anshul
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

#include <netinet/in.h>
/**
 * Create a Tap interface.
 */
int32_t tap_alloc(char *name, int flags)
{
  int32_t fd, err;
  struct ifreq ifr = {0};

  char *clonedev = "/dev/net/tun";

  /* Open the clone device */
  if((fd = open(clonedev, O_RDWR))<0)
  {
    goto EXIT_LABEL;
  }
  ifr.ifr_flags = flags;
  if(*name)
  {
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
  }

  /* Try to create a device */
  err = ioctl(fd, TUNSETIFF, (void *)&ifr);
  if(err <0)
  {
    close(fd);
    fd = -1;
    goto EXIT_LABEL;
  }

  strcpy(name, ifr.ifr_name);

EXIT_LABEL:
  return(fd);
}

struct pdu
{
    uint32_t length;
    char first_byte;
};

/**
 * main entry point into the function
 *
 * @param argc Number of arguments
 * @param argv [1] Listen Port [2] destination IP
 */
int main(int argc, char *argv[])
{
  /* Open a tap interface and start listening for data on this interface. */
  char tap_name[IFNAMSIZ];

  int32_t tap_fd, net_fd;

  struct sockaddr_in addr, src_addr, recv_addr;

  fd_set fds;
  int32_t num_fds;

  int32_t num_reads;
  char buf[1600];
  int32_t i;
  int32_t recv_len;
  uint32_t have_partner = 0;


  struct timeval select_timeout;

  if((argc < 2)|| (argc > 2 && argc < 4))
  {
    fprintf(stderr, "Invalid syntax\n"
                    "peer <ifname> <listen_port> [<destination IPv4 address> <destination port>]\n");
    goto EXIT_LABEL;
  }

  strcpy(tap_name, argv[1]);
  tap_fd = tap_alloc(tap_name, IFF_TUN | IFF_NO_PI);
  if(tap_fd <0 )
  {
    fprintf(stderr, "Error creating tap interface.\n");
    fprintf(stderr, "Are you running as sudo?\n");
    goto EXIT_LABEL;
  }
  if(ioctl(tap_fd, TUNSETPERSIST, 0)<0)
  {
    perror("enabling TUNSETPERSIST");
    fprintf(stderr, "Are you running as sudo?\n");
    goto EXIT_LABEL;
  }

  /**
   * Open a UDP socket to send what we received on tun interface to application
   */
  net_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(net_fd < 0)
  {
    fprintf(stderr, "Error opening UDP socket.\n");
    perror("Cause");
    goto EXIT_LABEL;
  }

  /* Local address */
  memset(&src_addr, 0, sizeof(struct sockaddr));
  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Bind to %d\n", atoi(argv[2]));
  src_addr.sin_port = htons(atoi(argv[2]));

  bind(net_fd, (struct sockaddr *) &src_addr, (socklen_t) sizeof(struct sockaddr_in));
  listen(net_fd, 5/* Max connections */);

  if(argc > 2)
  {
    /* Destination Address */
    addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, (const char *)argv[3], (void *) &addr.sin_addr.s_addr)!=1)
    {
      fprintf(stderr, "Error converting destination IP address to a valid byte format.\n");
      goto EXIT_LABEL;
    }
    addr.sin_port = htons(atoi(argv[4]));
    have_partner = 1;

    char tmp_buf[15] = "hello world!";
    if(sendto(net_fd, tmp_buf, strlen(tmp_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr))<strlen(tmp_buf))
    {
      fprintf(stderr, "Could not send all data to remote.\n");
      goto EXIT_LABEL;
    }
  }

  while(1)
  {
    FD_ZERO(&fds);
    FD_SET(tap_fd, &fds);
    FD_SET(net_fd, &fds);

    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 250000; /* 250ms */
    num_fds  = select((tap_fd > net_fd? tap_fd:net_fd) +1, &fds, NULL, NULL, &select_timeout);
    if(FD_ISSET(tap_fd, &fds))
    {
      num_reads = read(tap_fd, buf, sizeof(buf));
      fprintf(stdout, "\n");
      for(i=0;i<num_reads;i++)
      {
        fprintf(stdout, "%c", buf[i]);
      }
      /* Encapsulate to remote */
      if(have_partner)
      {
        if(sendto(net_fd, buf, num_reads, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr))<num_reads)
        {
          fprintf(stderr, "Could not send all data to remote.\n");
          goto EXIT_LABEL;
        }
      }
    }
    if(FD_ISSET(net_fd, &fds))
    {
      fprintf(stdout, "Incoming from network.\n");
      memset(&recv_addr, 0, sizeof(struct sockaddr_in));
      num_reads = recvfrom(net_fd, buf, sizeof(buf), 0, (struct sockaddr *)&recv_addr, &recv_len);

      if(!have_partner)
      {
        /* Now we have a partner */
        fprintf(stdout, "New Partner\n");
        have_partner = 1;
        memcpy(&addr, &recv_addr, sizeof(struct sockaddr));
      }
      /* Write to tap interface */
      write(tap_fd, buf, num_reads);
    }
  }

EXIT_LABEL:
  return(0);
}
