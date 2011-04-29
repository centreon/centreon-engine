/*
** Copyright 2011      Merethis
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

#ifndef CCE_LOGGING_OBJECT_HH
# define CCE_LOGGING_OBJECT_HH

namespace                 com {
  namespace               centreon {
    namespace             engine {
      namespace           logging {
	/**
	 *  @class object object.hh
	 *  @brief Parent class of all logging objects.
	 *
	 *  Parent class of all logging objects.
	 */
	class             object {
	public:
	  /**
	   *  @enum object::e_type
	   *  Logging types.
	   */
	  enum            e_type {
	    none                 = 0ull,

	    runtime_error        = 1ull,
	    runtime_warning      = 2ull,

	    verification_error   = 4ull,
	    verification_warning = 8ull,

	    config_error         = 16ull,
	    config_warning       = 32ull,

	    process_info         = 64ull,
	    event_handler        = 128ull,
	    // notification         = 256ull, //NOT USED ANYMORE - CAN BE REUSED
	    external_command     = 512ull,

	    host_up              = 1024ull,
	    host_down            = 2048ull,
	    host_unreachable     = 4096ull,

	    service_ok           = 8192ull,
	    service_unknown      = 16384ull,
	    service_warning      = 32768ull,
	    service_critical     = 65536ull,

	    passive_check        = 131072ull,

	    info_message         = 262144ull,

	    host_notification    = 524288ull,
	    service_notification = 1048576ull,

	    dbg_functions        = 1ull << 32,
	    dbg_config           = 2ull << 32,
	    dbg_process          = 4ull << 32,
	    dbg_statusdata       = 4ull << 32,
	    dbg_retentiondata    = 4ull << 32,
	    dbg_events           = 8ull << 32,
	    dbg_checks           = 16ull << 32,
	    dbg_ipc              = 16ull << 32,
	    dbg_flapping         = 16ull << 32,
	    dbg_eventhandlers    = 16ull << 32,
	    dbg_perfdata         = 16ull << 32,
	    dbg_notifications    = 32ull << 32,
	    dbg_eventbroker      = 64ull << 32,
	    dbg_externalcommands = 128ull << 32,
	    dbg_commands         = 256ull << 32,
	    dbg_downtime         = 512ull << 32,
	    dbg_comments         = 1024ull << 32,
	    dbg_macros           = 2048ull << 32,

	    dbg_all              = 4095ull << 32
	  };

	  /**
	   *  @enum object::e_verbose
	   *  Logging verbosity.
	   */
	  enum            e_verbosity {
	    basic = 0u,
	    more  = 1u,
	    most  = 2u
	  };

	  virtual         ~object() throw() = 0;

	  virtual void    log(char const* message,
			      unsigned long long type,
			      unsigned int verbosity) throw() = 0;
	};

	inline object::~object() throw() {}
      }
    }
  }
}

#endif // !CCE_LOGGING_OBJECT_HH
