/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2003 Mikael Hallendal <micke@codefactory.se>
 * Copyright (C) 2003 CodeFactory AB. 
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

#ifndef __LM_CONNECTION_H__
#define __LM_CONNECTION_H__

#if !defined (LM_INSIDE_LOUDMOUTH_H) && !defined (LM_COMPILATION)
#error "Only <loudmouth/loudmouth.h> can be included directly, this file may di\sappear or change contents."
#endif

#include <loudmouth/lm-message.h>

#define LM_CONNECTION(o) (LmConnection *) o;

#define LM_CONNECTION_DEFAULT_PORT     5222
#define LM_CONNECTION_DEFAULT_PORT_SSL 5223

typedef struct _LmConnection LmConnection;

typedef struct LmMessageHandler LmMessageHandler;

typedef enum {
	LM_HANDLER_RESULT_REMOVE_MESSAGE,
	LM_HANDLER_RESULT_ALLOW_MORE_HANDLERS
} LmHandlerResult;
 
typedef enum {
	LM_HANDLER_PRIORITY_LAST   = 1,
	LM_HANDLER_PRIORITY_NORMAL = 2,
	LM_HANDLER_PRIORITY_FIRST  = 3
} LmHandlerPriority;

typedef enum {
	LM_DISCONNECT_REASON_OK,
	LM_DISCONNECT_REASON_PING_TIME_OUT,
	LM_DISCONNECT_REASON_HUP,
	LM_DISCONNECT_REASON_ERROR,
	LM_DISCONNECT_REASON_UNKNOWN
} LmDisconnectReason;

typedef void (* LmResultFunction)     (LmConnection *connection,
				       gboolean      success,
				       gpointer      user_data);

typedef void (* LmDisconnectFunction) (LmConnection *connection,
				       LmDisconnectReason reason,
				       gpointer           user_data);


LmConnection *lm_connection_new               (const gchar        *server);
gboolean      lm_connection_open              (LmConnection       *connection,
					       LmResultFunction    function,
					       gpointer            user_data,
					       GDestroyNotify      notify,
					       GError            **error);
gboolean      lm_connection_open_and_block    (LmConnection       *connection,
					       GError            **error);

gboolean      lm_connection_close             (LmConnection       *connection,
					       GError            **error);
gboolean      lm_connection_authenticate      (LmConnection       *connection,
					       const gchar        *username,
					       const gchar        *password,
					       const gchar        *resource,
					       LmResultFunction    function,
					       gpointer            user_data,
					       GDestroyNotify      notify,
					       GError            **error);
gboolean
lm_connection_authenticate_and_block          (LmConnection       *connection,
					       const gchar        *username,
					       const gchar        *password,
					       const gchar        *resource,
					       GError            **error);
gboolean      lm_connection_is_open           (LmConnection       *connection);
gboolean      lm_connection_is_authenticated  (LmConnection       *connection);

const gchar * lm_connection_get_server        (LmConnection       *connection);
void          lm_connection_set_server        (LmConnection       *connection,
					       const gchar        *server);

guint         lm_connection_get_port          (LmConnection       *connection);
void          lm_connection_set_port          (LmConnection       *connection,
					       guint               port);
gboolean      lm_connection_get_use_ssl       (LmConnection       *connection);
void          lm_connection_set_use_ssl       (LmConnection       *connection,
					       gboolean            use_ssl);
					       
gboolean      lm_connection_send              (LmConnection       *connection,
					       LmMessage          *message,
					       GError            **error);
gboolean      lm_connection_send_with_reply   (LmConnection       *connection,
					       LmMessage          *message,
					       LmMessageHandler   *handler,
					       GError            **error);
LmMessage *   
lm_connection_send_with_reply_and_block       (LmConnection       *connection,
					       LmMessage          *message,
					       GError            **error);
void
lm_connection_register_message_handler        (LmConnection       *connection,
					       LmMessageHandler   *handler,
					       LmMessageType       type,
					       LmHandlerPriority   priority);
void
lm_connection_unregister_message_handler      (LmConnection       *connection,
					       LmMessageHandler   *handler,
					       LmMessageType       type);
void 
lm_connection_set_disconnect_function         (LmConnection       *connection,
					       LmDisconnectFunction function,
					       gpointer             user_data,
					       GDestroyNotify       notify);
					       
gboolean      lm_connection_send_raw          (LmConnection       *connection,
					       const gchar        *str,
					       GError            **error);
LmConnection* lm_connection_ref               (LmConnection       *connection);
void          lm_connection_unref             (LmConnection       *connection);


#endif /* __LM_CONNECTION_H__ */
