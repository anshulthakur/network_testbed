/*
 * util.c
 *
 *  Created on: 01-Mar-2017
 *      Author: anshul
 */

#include <stdio.h>                        /* Defines perror() */
#include <stdlib.h>
#include <unistd.h>                       /* Defines close() */
#include <sys/socket.h>
#include <arpa/inet.h>                    /* inet_pton() */
#include <logs.h>

typedef enum socket_types
{
  SOCK_TYPE_TCP_LISTEN,
  SOCK_TYPE_TCP_INCOMING,
  SOCK_TYPE_TCP_OUTGOING,
  SOCK_TYPE_UDP,
  SOCK_TYPE_MULTICAST,

  __MAX_SOCK_TYPES__
} SOCK_TYPE;

typedef struct connection_params
{
    /**
     * @brief String representation of IPv4 Address of destination or bind address.
     */
    union address
    {
        /**
         * @brief For Listen sockets, we call it bind address
         */
        char bind_addr[16];
        /**
         * @brief for Incoming Connections, we call it source address
         */
        char src_addr[16];
    } addr;

}CONNECTION_PARAMS;

typedef struct socket_cb
{
    /**
     * @brief Address to bind/originator of request.
     */
    union address
    {
        /**
         * @brief For Listen sockets, we call it bind address
         */
        struct sockaddr bind_addr;
        /**
         * @brief for Incoming Connections, we call it source address
         */
        struct sockaddr src_addr;
    } addr;

    /**
     * @brief File Descriptor
     */
    int fd;
} SOCKET_CB;

/**
 * @brief Create Socket API
 * @param type Type of Socket requested
 * @param params Parameters for socket
 */
SOCKET_CB* util_socket_create(SOCK_TYPE type, CONNECTION_PARAMS *params)
{
  SOCKET_CB* sock_cb=NULL;
  int ret_val = 0;

  TRACE_ENTRY();

  sock_cb = (SOCKET_CB *)malloc(sizeof(SOCKET_CB));
  if(sock_cb == NULL)
  {
    TRACE_ERROR("Memory allocation for Socket CB Failed!\n");
    goto EXIT_LABEL;
  }
  memset(sock_cb, 0, sizeof(SOCKET_CB));

  switch(type)
  {
    case SOCK_TYPE_TCP_LISTEN:
      if((ret_val = inet_pton(AF_INET, (const char *)params->addr, (void *) &sock_cb->addr))!= 1)
      {
        if (ret_val==0)
        {
          TRACE_ERROR("Invalid Address Representation.");
        }
        else
        {
          perror("Error: ");
        }
        free(sock_cb);
        sock_cb = NULL;
        goto EXIT_LABEL;
      }
      sock_cb->fd = socket(AF_INET, SOCK_STREAM, PF_INET);
      if(sock_cb->fd <= 0)
      {
        TRACE_ERROR("Error opening socket!");
        free(sock_cb);
        sock_cb = NULL;
        goto EXIT_LABEL;
      }
  }
EXIT_LABEL:
  TRACE_EXIT();
  return sock_cb;
}
