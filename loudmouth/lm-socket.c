/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2006 Imendio AB
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "lm-socket.h"

#ifndef G_OS_WIN32
typedef int LmSock;
#else  /* G_OS_WIN32 */
typedef SOCKET LmSock;
#endif /* G_OS_WIN32 */

/* FIXME: Integrate with the SSL stuff */

struct _LmSocket {
	LmSock          sock;
	GIOChannel     *channel;
	/* FIXME: Add the rest */

	LmSSL          *ssl;
	
	LmSocketFuncs   funcs;
	
	LmSocketState   state;

	gboolean        is_blocking;
	
	gint            ref;
};

static LmSocket *  socket_create (void);
static void        socket_free   (LmSocket *socket);

static LmSocket *
socket_create (void)
{
	LmSocket *socket;

	socket = g_new0 (LmSocket, 1);
	socket->ref_count = 1;
	socket->is_blocking = FALSE;
	socket->state = LM_SOCKET_STATE_CLOSED;
	socket->ssl = NULL;
	
	return socket;
}

static void
socket_free (LmSocket *socket)
{
	/* FIXME: Free up the rest of the memory */

	if (socket->ssl) {
		_lm_ssl_unref (socket->ssl);
	}

	g_free (socket->host);

	g_free (socket);
}

LmSocket *
lm_socket_new (LmSocketFuncs funcs, const gchar *host, guint port)
{
	LmSocket *socket;

	socket = socket_create ();

	socket->funcs = funcs;
	socket->host = g_strdup (host);
	socket->port = port;
	socket->is_blocking = FALSE;

	return socket;
}

void
lm_socket_open (LmSocket *socket)
{
	g_return_if_fail (socket != NULL);

	/* Fork and DNS Lookup */
}

int
lm_socket_get_fd (LmSocket *socket)
{
	g_return_val_if_fail (socket != NULL, -1);

	return socket->fd;
}

gboolean
lm_socket_get_is_blocking (LmSocket *socket)
{
	return socket->is_blocking;
}

void
lm_socket_set_is_blocking (LmSocket *socket, gboolean is_block)
{
	int res;

	g_return_if_fail (socket != NULL);

	/* FIXME: Don't unset all flags */

#ifndef G_OS_WIN32
	res = fcntl (socket->sock, F_SETFL, is_block ? 0 : O_NONBLOCK);
#else  /* G_OS_WIN32 */
	u_long mode = (is_block ? 0 : 1);
	res = ioctlsocket (socket->sock, FIONBIO, &mode);
#endif /* G_OS_WIN32 */

	if (res != 0) {
		g_log (LM_LOG_DOMAIN, LM_LOG_LEVEL_NET,
		       "Could not set socket to be %s\n",
		       block ? "blocking" : "non-blocking");
	}

	socket->is_blocking = is_block;
}

int
lm_socket_write (LmSocket   *socket,
		 gsize       size,
		 gchar      *buf,
		 GError    **error)
{
	gint b_written;
	
	g_return_val_if_fail (socket != NULL, -1);

	if (socket->ssl) {
		b_written = _lm_ssl_send (socket->ssl, buf, len);
	} else {
		GIOStatus io_status = G_IO_STATUS_AGAIN;
		gsize     bytes_written = 0;

		while (io_status == G_IO_STATUS_AGAIN) {
			io_status = g_io_channel_write_chars (socket->channel,
							      buf, size,
							      &bytes_written,
							      NULL);
		}

		b_written = bytes_written;

		if (io_status != G_IO_STATUS_NORMAL) {
			b_written = -1;
		}
	}

	return b_written;
}

int
lm_socket_read (LmSocket   *socket,
		gsize       size,
		gchar      *buf,
		GError    **error)
{
	gsize     bytes_read = 0;
	GIOStatus status =  G_IO_STATUS_AGAIN;

	g_return_val_if_fail (socket != NULL, -1);

	while (status == G_IO_STATUS_AGAIN) {
		if (socket->ssl) {
			status = _lm_ssl_read (socket->ssl, 
					       buf, size, &bytes_read);
		} else {
			status = g_io_channel_read_chars (socket->channel, 
							  buf, size, 
							  &bytes_read,
							  NULL);
		}
	}

	if (status != G_IO_STATUS_NORMAL || bytes_read < 0) {
		/* FIXME: Set error */

		return -1;
	}

	return bytes_read;
}

gboolean
lm_socket_close (LmSocket *socket, GError **error)
{
#ifndef G_OS_WIN32
	close (socket->sock);
#else /* G_OS_WIN32 */
	closesocket (socket->sock);
#endif /* G_OS_WIN32 */
}

LmSocket *
lm_socket_ref (LmSocket *socket)
{
	g_return_val_if_fail (socket != NULL, NULL);

	socket->ref++;
	
	return socket;
}

void
lm_socket_unref (LmSocket *socket)
{
	g_return_if_fail (socket != NULL);
	
	socket->ref--;
	
	if (socket->ref <= 0) {
		socket_free (socket);
	}
}
