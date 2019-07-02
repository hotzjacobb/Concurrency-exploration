/*
 * ws protocol handler plugin for "lws-minimal"
 * Server to forward mpi data
 *
 *
 * Made referencing the libwebsocket minimal web server
 *
 * http server with lws,
 * with an added websocket chat server.
 * 
 *
 *
 * This version holds a single message at a time
 */

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>


#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>


#ifndef ADDRESS
#define ADDRESS "127.0.0.1"
#endif


#ifndef PORT
#define PORT 5555
#endif

/* data for connecting to MPI adapter */

static struct lws_context *context_mpi;
static struct lws *client_wsi;
static int interrupted, zero_length_ping, port = 5555,  // port that adapter listens on
	   ssl_connection = !LCCSCF_USE_SSL;
static const char *server_address = "localhost", *pro = "lws-minimal";

/* fd for socket that sends messages to MPI */

// int conn_fd;
// int conn_fd_init = 0;

/* one of these created for each message */

struct msg {
	void *payload; /* is malloc'd */
	size_t len;
};

/* one of these is created for each client connecting to us */

struct per_session_data__minimal {
	struct per_session_data__minimal *pss_list;
	struct lws *wsi;
	int last; /* the last message number we sent */
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;

	struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

	struct msg amsg; /* the one pending message... */
	int current; /* the current message number we are caching */
};

/* destroys the message when everyone has had a copy of it */

static void
__minimal_destroy_message(void *_msg)
{
	struct msg *msg = _msg;

	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}

static int
callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{



	struct per_session_data__minimal *pss =
			(struct per_session_data__minimal *)user;
	struct per_vhost_data__minimal *vhd =
			(struct per_vhost_data__minimal *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi),
					lws_get_protocol(wsi));

					
	int m;

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
				lws_get_protocol(wsi),
				sizeof(struct per_vhost_data__minimal));
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);

		// // Just TCP no websocket

		// int ret = 0;
		// struct sockaddr_in server_addr = { 0 };

		// server_addr.sin_family = AF_INET;

		
		// server_addr.sin_port = htons(PORT);

		
		// ret = inet_pton(AF_INET, ADDRESS, &server_addr.sin_addr);
		// if (ret != 1) {
		// 	if (ret == -1) {
		// 		perror("inet_pton");
		// 	}
		// 	fprintf(stderr,
		// 			"failed to convert address %s "
		// 			"to binary net address\n",
		// 			ADDRESS);
		// 	return -1;
		// }

		// fprintf(stdout, "CONNECTING: address=%s port=%d\n", ADDRESS, PORT);

		
		// conn_fd = socket(AF_INET, SOCK_STREAM, 0);
		// if (conn_fd == -1) {
		// 	perror("socket");
		// 	return -1;
		// }

		
		// ret = connect(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
		// if (ret == -1) {
		// 	perror("connect");
		// 	return -1;
		// }

		// conn_fd_init = 1;

		break;

	case LWS_CALLBACK_ESTABLISHED:
		/* add ourselves to the list of live pss held in the vhd */
		lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
		pss->wsi = wsi;
		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_CLOSED:
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
				  pss, vhd->pss_list);

		// int ret = shutdown(conn_fd, SHUT_RDWR);
		// if (ret == -1) {
		// 	perror("shutdown");
		// 	return -1;
		// }

		
		// ret = close(conn_fd);
		// if (ret == -1) {
		// 	perror("close");
		// 	return -1;
		// }
		// break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (!vhd->amsg.payload)
			break;

		if (pss->last == vhd->current)
			break;

		
		/* notice we allowed for LWS_PRE in the payload already */
		m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) +
			      LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
		if (m < (int)vhd->amsg.len) {
			lwsl_err("ERROR %d writing to ws\n", m);
			return -1;
		}

		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_RECEIVE:

		
		if (vhd->amsg.payload)
			__minimal_destroy_message(&vhd->amsg);

		vhd->amsg.len = len;
		/* notice we over-allocate by LWS_PRE */
		vhd->amsg.payload = malloc(LWS_PRE + len);
		if (!vhd->amsg.payload) {
			lwsl_user("OOM: dropping\n");
			break;
		}

	

		

		memcpy((char *)vhd->amsg.payload + LWS_PRE, in, len);
		vhd->current++;

		printf("in: %s\r\n", vhd->amsg.payload + LWS_PRE);

		char* buff;
		if((buff = malloc(strlen(in)+strlen("\r\n"))) != NULL){
		
	
    	buff[0] = '\0';   // ensures the memory is an empty string
			strcat(buff,in);
			strcat(buff,"\r\n");
		} else {
			return -1;
		}


		free(buff);
		memset(in, 0, strlen(in) + 1);       // reset so clean for next




			/*
			* let everybody know we want to write something on them
			* as soon as they are ready
			*/
			lws_start_foreach_llp(struct per_session_data__minimal **,
						ppss, vhd->pss_list) {
				lws_callback_on_writable((*ppss)->wsi);
			} lws_end_foreach_llp(ppss, pss_list);
			break;

	default:
		break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL \
	{ \
		"lws-minimal", \
		callback_minimal, \
		sizeof(struct per_session_data__minimal), \
		128, \
		0, NULL, 0 \
	}

#if !defined (LWS_PLUGIN_STATIC)

/* boilerplate needed if we are built as a dynamic plugin */

static const struct lws_protocols protocols[] = {
	LWS_PLUGIN_PROTOCOL_MINIMAL
};

LWS_EXTERN LWS_VISIBLE int
init_protocol_minimal(struct lws_context *context,
		      struct lws_plugin_capability *c)
{
	if (c->api_magic != LWS_PLUGIN_API_MAGIC) {
		lwsl_err("Plugin API %d, library API %d", LWS_PLUGIN_API_MAGIC,
			 c->api_magic);
		return 1;
	}

	c->protocols = protocols;
	c->count_protocols = LWS_ARRAY_SIZE(protocols);
	c->extensions = NULL;
	c->count_extensions = 0;

	return 0;
}

LWS_EXTERN LWS_VISIBLE int
destroy_protocol_minimal(struct lws_context *context)
{
	return 0;
}
#endif
