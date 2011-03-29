/*
** Copyright 2000-2006 Ethan Galstad
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

/*********** COMMON HEADER FILES ***********/

#include "conf.hh"
#ifdef USE_XSDDEFAULT
# include "../xdata/xsddefault.hh"		/* default routines */
#endif
#include "broker.hh"
#include "configuration.hh"
#include "statusdata.hh"

extern com::centreon::scheduler::configuration config;

/******************************************************************/
/****************** TOP-LEVEL OUTPUT FUNCTIONS ********************/
/******************************************************************/

/* initializes status data at program start */
int initialize_status_data(char *config_file){
	int result=OK;

	/**** IMPLEMENTATION-SPECIFIC CALLS ****/
#ifdef USE_XSDDEFAULT
	result=xsddefault_initialize_status_data(config_file);
#endif

	return result;
        }


/* update all status data (aggregated dump) */
int update_all_status_data(void){
	int result=OK;

#ifdef USE_EVENT_BROKER
	/* send data to event broker */
	broker_aggregated_status_data(NEBTYPE_AGGREGATEDSTATUS_STARTDUMP,NEBFLAG_NONE,NEBATTR_NONE,NULL);
#endif

	/**** IMPLEMENTATION-SPECIFIC CALLS ****/
#ifdef USE_XSDDEFAULT
	result=xsddefault_save_status_data();
#endif

#ifdef USE_EVENT_BROKER
	/* send data to event broker */
	broker_aggregated_status_data(NEBTYPE_AGGREGATEDSTATUS_ENDDUMP,NEBFLAG_NONE,NEBATTR_NONE,NULL);
#endif

	if(result!=OK)
		return ERROR;

	return OK;
        }


/* cleans up status data before program termination */
int cleanup_status_data(char *config_file,int delete_status_data){
	int result=OK;

	/**** IMPLEMENTATION-SPECIFIC CALLS ****/
#ifdef USE_XSDDEFAULT
	result=xsddefault_cleanup_status_data(config_file,delete_status_data);
#endif

	return result;
        }



/* updates program status info */
int update_program_status(int aggregated_dump){

#ifdef USE_EVENT_BROKER
	/* send data to event broker (non-aggregated dumps only) */
	if(aggregated_dump==FALSE)
	        broker_program_status(NEBTYPE_PROGRAMSTATUS_UPDATE,NEBFLAG_NONE,NEBATTR_NONE,NULL);
#endif

	return OK;
        }



/* updates host status info */
int update_host_status(host *hst,int aggregated_dump){

#ifdef USE_EVENT_BROKER
	/* send data to event broker (non-aggregated dumps only) */
	if(aggregated_dump==FALSE)
	          broker_host_status(NEBTYPE_HOSTSTATUS_UPDATE,NEBFLAG_NONE,NEBATTR_NONE,hst,NULL);
#endif

	return OK;
        }



/* updates service status info */
int update_service_status(service *svc,int aggregated_dump){

#ifdef USE_EVENT_BROKER
	/* send data to event broker (non-aggregated dumps only) */
	if(aggregated_dump==FALSE)
	        broker_service_status(NEBTYPE_SERVICESTATUS_UPDATE,NEBFLAG_NONE,NEBATTR_NONE,svc,NULL);
#endif

	return OK;
        }



/* updates contact status info */
int update_contact_status(contact *cntct,int aggregated_dump){

#ifdef USE_EVENT_BROKER
	/* send data to event broker (non-aggregated dumps only) */
	if(aggregated_dump==FALSE)
	        broker_contact_status(NEBTYPE_CONTACTSTATUS_UPDATE,NEBFLAG_NONE,NEBATTR_NONE,cntct,NULL);
#endif

	return OK;
        }
