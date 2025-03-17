#pragma once

#include "xess_debug.h"

xess_result_t hk_xessSelectNetworkModel(xess_context_handle_t hContext, xess_network_model_t network);
xess_result_t hk_xessStartDump(xess_context_handle_t hContext, const xess_dump_parameters_t* dump_parameters);
