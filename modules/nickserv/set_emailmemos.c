/*
 * Copyright (c) 2005 William Pitcock <nenolod -at- nenolod.net>
 * Copyright (c) 2007 Jilles Tjoelker
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Forwards incoming memos to your e-mail address.
 *
 */

#include "atheme.h"
#include "uplink.h"
#include "list_common.h"
#include "list.h"

DECLARE_MODULE_V1
(
	"nickserv/set_emailmemos", false, _modinit, _moddeinit,
	PACKAGE_STRING,
	"Atheme Development Group <http://www.atheme.org>"
);

mowgli_patricia_t **ns_set_cmdtree;

static void ns_cmd_set_emailmemos(sourceinfo_t *si, int parc, char *parv[]);

command_t ns_set_emailmemos = { "EMAILMEMOS", N_("Forwards incoming memos to your e-mail address."), AC_NONE, 1, ns_cmd_set_emailmemos, { .path = "nickserv/set_emailmemos" } };

static bool has_emailmemos(const mynick_t *mn, const void *arg)
{
	myuser_t *mu = mn->owner;

	return ( mu->flags & MU_EMAILMEMOS ) == MU_EMAILMEMOS;
}

void _modinit(module_t *m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, ns_set_cmdtree, "nickserv/set_core", "ns_set_cmdtree");

	command_add(&ns_set_emailmemos, *ns_set_cmdtree);

	use_nslist_main_symbols(m);

	static list_param_t emailmemos;
	emailmemos.opttype = OPT_BOOL;
	emailmemos.is_match = has_emailmemos;

	list_register("emailmemos", &emailmemos);
}

void _moddeinit(module_unload_intent_t intent)
{
	command_delete(&ns_set_emailmemos, *ns_set_cmdtree);
	list_unregister("emailmemos");
}

/* SET EMAILMEMOS [ON|OFF] */
static void ns_cmd_set_emailmemos(sourceinfo_t *si, int parc, char *parv[])
{
	char *params = parv[0];

	if (si->smu->flags & MU_WAITAUTH)
	{
		command_fail(si, fault_noprivs, _("You have to verify your email address before you can enable emailing memos."));
		return;
	}

	if (!params)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "EMAILMEMOS");
		return;
	}

	if (!strcasecmp("ON", params))
	{
		if (me.mta == NULL)
		{
			command_fail(si, fault_emailfail, _("Sending email is administratively disabled."));
			return;
		}
		if (MU_EMAILMEMOS & si->smu->flags)
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is already set for account \2%s\2."), "EMAILMEMOS", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:EMAILMEMOS:ON");
		si->smu->flags |= MU_EMAILMEMOS;
		command_success_nodata(si, _("The \2%s\2 flag has been set for account \2%s\2."), "EMAILMEMOS", entity(si->smu)->name);
		return;
	}

	else if (!strcasecmp("OFF", params))
	{
		if (!(MU_EMAILMEMOS & si->smu->flags))
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is not set for account \2%s\2."), "EMAILMEMOS", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:EMAILMEMOS:OFF");
		si->smu->flags &= ~MU_EMAILMEMOS;
		command_success_nodata(si, _("The \2%s\2 flag has been removed for account \2%s\2."), "EMAILMEMOS", entity(si->smu)->name);
		return;
	}
	else
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "EMAILMEMOS");
		return;
	}
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */
