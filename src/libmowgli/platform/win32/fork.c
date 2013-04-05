/*
 * libmowgli: A collection of useful routines for programming.
 * fork.c: Fastest possible NT fork() implementation
 *
 * Copyright (c) 2012 TortoiseLabs, LLC.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * References:
 *  - undocumented.ntinternals.net
 *  - http://www.cs.miami.edu/~burt/journal/NT/processinit.html
 */

#include "mowgli.h"

#ifdef NOTYET

# ifdef _WIN32

#  ifndef _WIN32_WINNT

int
fork(void)
{
#   warning fork is not possible on your platform in any sane way, sorry :(
	return -ENOSYS;
}

#  else

extern NTSTATUS NTAPI CsrCallClientServer(void *message, void *userdata, uint32_t opcode, uint32_t size);

/*
 * Definition of a message sent to an NT port on the CSRSS server.
 *
 * Not sure what dummy1/dummy2 do, but they're junk as far as I can see.
 */
struct csrss_message
{
	uint32_t dummy1;
	uint32_t opcode;
	uint32_t status;
	uint32_t dummy2;
};

static inline void
inherit_handles(void)
{
	uint32_t n = 0x1000;
	uint32_t *p = mowgli_alloc_array(sizeof(uint32_t), n);
	uint32_t pid, i;
	SYSTEM_HANDLE_INFORMATION *info;

	while (ZwQuerySystemInformation(SystemHandleInformation, p, n * sizeof(*p), 0) == STATUS_INFO_LENGTH_MISMATCH)
	{
		n *= 2;

		mowgli_free(p);
		p = mowgli_alloc_array(sizeof(uint32_t), n);
	}

	info = (SYSTEM_HANDLE_INFORMATION *) (p + 1);
	pid = GetCurrentProcessId();

	for (i = 0; i < *p; i++)
		if (info[i].ProcessId == pid)
			SetHandleInformation((HANDLE) h[i].Handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	mowgli_free(p);
}

static inline void
request_csrss_session(HANDLE proc_handle, HANDLE thread_handle, uint32_t pid, uint32_t tid)
{
	struct
	{
		PORT_MESSAGE port_message;
		struct csrss_message csrss_message;

		PROCESS_INFORMATION process_information;
		CLIENT_ID debugger;
		uint32_t flags;
		uint32_t vdminfo[2];
	} csrmsg =
	{
		{ 0 }, { 0 }, { proc_handle, thread_handle, pid, tid }, { 0 }, 0, { 0 }
	};

	CsrCallClientServer(&csrmsg, NULL, 0x10000, sizeof csrmsg);
}

int
child(void)
{
	typedef BOOL (*CsrpConnectToServer)(PWSTR);

	CsrpConnectToServer(0x77F8F65D) (L"\\Windows");
	__asm__("mov eax, 0; mov esp, ebp; pop ebp; ret");
}

int
fork(void)
{
	HANDLE proc_handle, thread_handle;
	OBJECT_ATTRIBUTES oa = { sizeof(oa) };
	CONTEXT context = { CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS | CONTEXT_FLOATING_POINT };
	MEMORY_BASIC_INFORMATION mbi;
	THREAD_BASIC_INFORMATION tbi;
	PNT_TIB tib;
	USER_STACK stack;
	CLIENT_ID cid;

	/* ensure the child has the same handles and ports */
	inherit_handles();

	/* create the actual LWP using ZwCreateProcess() */
	ZwCreateProcess(&proc_handle, PROCESS_ALL_ACCESS, &oa, NtCurrentProcess(), TRUE, 0, 0, 0);

	/* now set up a thread for that process using a context, cloning the current thread ... */
	ZwGetContextThread(NtCurrentThread(), &context);
	context.Eip = (unsigned long) child;

	/* set up a stack for the thread now that the child sentinel is set up ... */
	ZwQueryVirtualMemory(NtCurrentProcess(), (void *) context.Esp, MemoryBasicInformation,
			     &mbi, sizeof mbi, 0);

	stack = (USER_STACK) { 0, 0, ((char *) mbi.BaseAddress) + mbi.RegionSize,
			       mbi.BaseAddress, mbi.AllocationBase };

	/* now spawn the thread! */
	ZwCreateThread(&thread_handle, THREAD_ALL_ACCESS, &oa, proc_handle, &cid, &context, &stack, TRUE);

	/* thread is spawned, but frozen for inspection -- fix up memory protection before unfreezing */
	ZwQueryInformationThread(NtCurrentThread(), ThreadBasicInformation, &tbi, sizeof tbi, 0);
	tib = tbi.TebBaseAddress;

	ZwQueryInformationThread(thread_handle, ThreadBasicInformation, &tbi, sizeof tbi, 0);
	ZwWriteVirtualMemory(process_handle, tbi.TebBaseAddress, &tib->ExceptionList, sizeof(tib->ExceptionList), 0);

	/* ready to go, now request a CSRSS session */
	request_csrss_session(process_handle, thread_handle, (uint32_t) cid.UniqueProcess, (uint32_t) cid.UniqueThread);

	/* CSRSS session set up or we segfaulted by now, so unfreeze the child... */
	ZwResumeThread(thread_handle, 0);

	/* release handle refcount now that process is freestanding */
	ZwClose(thread_handle);
	ZwClose(process_handle);

	return (int) cid.UniqueProcess;
}

#  endif

# endif

#endif
