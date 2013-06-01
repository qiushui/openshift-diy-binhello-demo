/* hello.c */

/* A microhttpd application to be run under the Red Hat OpenShift PaaS
   DIY cartridge.  Demonstrates running a binary application in this
   PaaS */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>

static int
answer_to_connection(void *cls,
		     struct MHD_Connection *connection,
		     const char *url,
		     const char *method,
		     const char *version,
		     const char *upload_data, size_t *upload_data_size,
		     void **con_cls)
{
  const char *page = "<html><body>Hello, World!</body></html>";
  struct MHD_Response *response;
  int ret;

  fprintf(stderr, "respond to %s %s\n", method, url);

  response =
    MHD_create_response_from_buffer(strlen(page), (void *)page, 
				    MHD_RESPMEM_PERSISTENT);
  ret =
    MHD_queue_response(connection, MHD_HTTP_OK, response);

  MHD_destroy_response(response);

  return ret;
}

int main (int argc, char *argv[])
{
  struct MHD_Daemon *daemon;
  struct sockaddr_in sad;
  unsigned short port;
  int ret;
  char *env_openshift_ipstr;
  char *env_openshift_port;

  env_openshift_ipstr = getenv("OPENSHIFT_INTERNAL_IP");
  if (NULL == env_openshift_ipstr) {
    env_openshift_ipstr = getenv("OPENSHIFT_DIY_IP");
  }
  env_openshift_port = getenv("OPENSHIFT_INTERNAL_PORT");
  if (NULL == env_openshift_port) {
    env_openshift_port = getenv("OPENSHIFT_DIY_PORT");
  }

  if (env_openshift_ipstr == NULL) {
    fprintf(stderr, "OPENSHIFT_DIY_IP is not defined\n");
    return 1;
  }
  if (env_openshift_port == NULL) {
    fprintf(stderr, "OPENSHIFT_DIY_PORT is not defined\n");
    return 1;
  }

  fprintf(stderr, "OPENSHIFT_DIY_IP is %s\n",  env_openshift_ipstr);
  fprintf(stderr, "OPENSHIFT_DIY_PORT is %s\n", env_openshift_port);

  port = (unsigned short) atoi(env_openshift_port);
  if (port == 0) {
    fprintf(stderr, "could not parse OPENSHIFT_INTERNAL_PORT\n");
    return 1;
  }

  sad.sin_family = AF_INET;
  sad.sin_port = htons(port);
  ret = inet_pton(AF_INET,
		  env_openshift_ipstr,
		  &(sad.sin_addr.s_addr));
  if (ret != 1) {
    fprintf(stderr, "could not parse OPENSHIFT_INTERNAL_ADDRESS\n");
    return 1;
  }

  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY
			    | MHD_USE_DEBUG,
			    htons(port),
			    NULL, NULL,
			    &answer_to_connection, NULL,
			    MHD_OPTION_SOCK_ADDR, (struct sockaddr *) &(sad),
			    MHD_OPTION_END);
  if (NULL == daemon) {
    fprintf(stderr, "fail MHD_start_daemon\n");
    return 1;
  }

  while (1) {  /* run forever, until killed */
    fprintf(stderr, "loop\n");
    sleep(60);
  }

  MHD_stop_daemon(daemon);
  return 0;
}
