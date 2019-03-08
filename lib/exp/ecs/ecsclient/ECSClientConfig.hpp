#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_EXP

#include <string>

const std::string DEFAULT_PROD_ECS_SERVER_URL_1 = "https://config.edge.skype.com/config/v1/";
const std::string DEFAULT_PROD_ECS_SERVER_URL_2 = "https://config.edge.skype.com/config/v1/";

const std::string DEFAULT_INT_ECS_SERVER_URL_1 = "https://config.edge.skype.net/config/v1/";
const std::string DEFAULT_INT_ECS_SERVER_URL_2 = "https://config.edge.skype.net/config/v1/";

const std::string  DEFAULT_CLIENT_NAME = "Skype";
const std::string  DEFAULT_CONFIG_ETAG = "\"\"";
const std::string  CONFIG_IDS_KEY = "ConfigIDs";
const std::string  DEFAULT_EVENT_TO_CONFIGIDS_MAPPING_AGENT_NAME = "EventToConfigIdsMapping";

const unsigned int DEFAULT_CONFIG_REFETCH_INTERVAL_IN_SECONDS = 3600;     // 1hr
const unsigned int DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN = 5 * 60;   // 5mins
const unsigned int DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX = 24 * 3600;// 24hrs

const std::string ECSCLIENT_CONFIG_UPDATE_SUCCEEDED = "Succeeded";
const std::string ECSCLIENT_CONFIG_UPDATE_FAILED = "Failed";
const std::string ECSCLIENT_CONFIG_UPDATE_TOBERETRIED = "ToBeRetried";

const std::string ECSCLIENT_CONFIG_SOURCE_SERVER = "Server";
const std::string ECSCLIENT_CONFIG_SOURCE_LOCAL = "Local";

const std::string ECSCLIENT_STATE_UNKNOWN = "Unknown";
const std::string ECSCLIENT_STATE_INITIALIZED = "Initialized";
const std::string ECSCLIENT_STATE_STARTED = "Started";
const std::string ECSCLIENT_STATE_SUSPENDED = "Suspended";
const std::string ECSCLIENT_STATE_STOPPING = "Stopping";
const std::string ECSCLIENT_STATE_STOPPED = "Stopped";
const std::string ECSCLIENT_STATE_REQUEST_PARAM_CHANGED = "RequestParameterChanged";

const std::string EVENT_FIELD_ECSCLIENT_CLIENTNAME = "ClientName";
const std::string EVENT_FIELD_ECSCLIENT_CLIENTVERSION = "ClientVersion";

const std::string EVENT_TYPE_ECSCLIENT_CONFIG_UPDATE = "ECSConfigUpdate";
const std::string EVENT_FIELD_ECSCLIENT_CONFIG_RESULT = "Result";
const std::string EVENT_FIELD_ECSCLIENT_CONFIG_SOURCE = "Source";

const std::string EVENT_TYPE_ECSCLIENT_STATE_CHANGE = "ECSClientState";
const std::string EVENT_FIELD_ECSCLIENT_STATE = "State";

#endif
