/*
** Copyright 2012 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"
#include "soapH.h"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules::webservice;

/**
 *  @brief Adds a comment to a particular host.
 *
 *  If the comment is not persistent, the comment will be deleted the
 *  next time Engine is restarted. Otherwise, the comment will persist
 *  across program restarts until it is deleted manually.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  comment Comment data.
 *  @param[out] res     New comment ID.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commentAddToHost(
      soap* s,
      ns1__hostIDType* host_id,
      ns1__commentType* comment,
      centreonengine__commentAddToHostResponse& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Insert new comment.
  unsigned long comment_id;
  if (add_new_comment(
        HOST_COMMENT,
        USER_COMMENT,
        host_id->name.c_str(),
        NULL,
        time(NULL),
        comment->author.c_str(),
        comment->text.c_str(),
        comment->persistent,
        COMMENTSOURCE_EXTERNAL,
        FALSE,
        0,
        &comment_id) < 0) {
    std::string* error(soap_new_std__string(s, 1));
    *error = "Could not add comment to host '" + host_id->name + "'";
    logger(log_runtime_error, more)
      << "Webservice: " << __func__ << " failed: " << *error;
    return (soap_receiver_fault(
              s,
              "Invalid parameter",
              error->c_str()));
  }

  // Return comment ID.
  res.commentid = soap_new_ns1__commentIDType(s, 1);
  res.commentid->comment = comment_id;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  @brief Adds a comment to a particular service.
 *
 *  If the comment is not persistent, the comment will be deleted the
 *  next time Engine is restarted. Otherwise, the comment will persist
 *  across program restarts until it is deleted manually.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  comment    Comment data.
 *  @param[out] res        New comment ID.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commentAddToService(
      soap* s,
      ns1__serviceIDType* service_id,
      ns1__commentType* comment,
      centreonengine__commentAddToServiceResponse& res) {
  // Begin try block.
  COMMAND_BEGIN(service_id->host << ", " << service_id->service)

  // Insert new comment.
  unsigned long comment_id;
  if (add_new_comment(
        SERVICE_COMMENT,
        USER_COMMENT,
        service_id->host->name.c_str(),
        service_id->service.c_str(),
        time(NULL),
        comment->author.c_str(),
        comment->text.c_str(),
        comment->persistent,
        COMMENTSOURCE_EXTERNAL,
        FALSE,
        0,
        &comment_id) < 0) {
    std::string* error(soap_new_std__string(s, 1));
    *error = "Could not add comment to service '"
      + service_id->service
      + "' of host '"
      + service_id->host->name
      + "'";
    logger(log_runtime_error, more)
      << "Webservice: " << __func__ << " failed: " << *error;
    return (soap_receiver_fault(
              s,
              "Invalid parameter",
              error->c_str()));
  }

  // Return comment ID.
  res.commentid = soap_new_ns1__commentIDType(s, 1);
  res.commentid->comment = comment_id;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Delete a comment.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  comment_id Target comment.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commentDelete(
      soap* s,
      ns1__commentIDType* comment_id,
      centreonengine__commentDeleteResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(comment_id->comment)

  // Delete comment.
  // XXX: don't know which one to delete so delete both
  delete_host_comment(comment_id->comment);
  delete_service_comment(comment_id->comment);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Delete all comments associated with a particular host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commentDeleteAllOfHost(
      soap* s,
      ns1__hostIDType* host_id,
      centreonengine__commentDeleteAllOfHostResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Delete comments.
  delete_all_comments(HOST_COMMENT, host_id->name.c_str(), NULL);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Delete all comments associated with a particular service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commentDeleteAllOfService(
      soap* s,
      ns1__serviceIDType* service_id,
      centreonengine__commentDeleteAllOfServiceResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(service_id->host->name << ", " << service_id->service)

  // Delete comments.
  delete_all_comments(
    SERVICE_COMMENT,
    service_id->host->name.c_str(),
    service_id->service.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}
