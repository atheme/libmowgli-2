/*
 * reslist.c - get nameservers from windows *
 *
 * Copyright 1998 by the Massachusetts Institute of Technology.
 * Copyright (C) 2007-2008 by Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include "mowgli.h"

#ifdef MOWGLI_OS_WIN
# include <windows.h>
# include <iphlpapi.h>

int
mowgli_dns_get_windows_nameservers(char *ret_buf, size_t ret_size)
{
	FIXED_INFO *fixedinfo, tfixedinfo;
	DWORD size = sizeof(*fixedinfo);

	typedef DWORD (WINAPI * get_net_param_func)(FIXED_INFO *, DWORD *);
	get_net_param_func get_network_params;
	HMODULE handle;
	IP_ADDR_STRING *ip_addr;
	int i, count = 0;
	size_t ip_size = sizeof("255.255.255.255");
	size_t left = ret_size;
	char *ret = ret_buf;
	HRESULT res;

	if (!(handle = LoadLibrary("iphlpapi.dll")))
		return 0;

	if (!(get_network_params = (get_net_param_func) GetProcAddress(handle, "GetNetworkParams")))
		goto quit;

	res = (*get_network_params)(&tfixedinfo, &size);

	if ((res != ERROR_BUFFER_OVERFLOW) && (res != ERROR_SUCCESS))
		goto quit;

	fixedinfo = mowgli_alloc(size);

	if (!fixedinfo || ((*get_network_params)(fixedinfo, &size) != ERROR_SUCCESS))
		goto quit;

# ifdef DEBUG
	mowgli_log("Host Name: %s\n", fixedinfo->HostName);
	mowgli_log("Domain Name: %s\n", fixedinfo->DomainName);
	mowgli_log("DNS Servers:\n\t%s (primary)\n", fixedinfo->DnsServerList.IpAddress.String);
# endif

	if ((strlen(fixedinfo->DnsServerList.IpAddress.String) > 0) &&
	    (inet_addr(fixedinfo->DnsServerList.IpAddress.String) != INADDR_NONE) && (left > ip_size))
	{
		ret += sprintf(ret, "%s,", fixedinfo->DnsServerList.IpAddress.String);
		left -= ret - ret_buf;
		count++;
	}

	for (i = 0, ip_addr = fixedinfo->DnsServerList.Next; ip_addr && left > ip_size; ip_addr = ip_addr->Next, i++)
	{
		if (inet_addr(ip_addr->IpAddress.String) != INADDR_NONE)
		{
			ret += sprintf(ret, "%s,", ip_addr->IpAddress.String);
			left -= ret - ret_buf;
			count++;
		}

# ifdef DEBUG
		mowgli_log("\t%s (secondary %d)\n", ip_addr->IpAddress.String, i + 1);
# endif
	}

	mowgli_free(fixedinfo);

quit:

	if (handle)
		FreeLibrary(handle);

	if (left <= ip_size)
		mowgli_log("Too many nameservers. Truncating to %d addressess", count);

	if (ret > ret_buf)
		ret[-1] = '\0';

	return count;
}

#endif
